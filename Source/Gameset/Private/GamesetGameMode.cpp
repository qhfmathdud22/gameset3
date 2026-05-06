#include "GamesetGameMode.h"
#include "GamesetSaveGame.h"          // 세이브 게임 클래스
#include "DeliveryManagerComponent.h"
#include "DeliveryComponent.h"        // 소지금·허기·갈증·GameOver 델리게이트
#include "UpgradeComponent.h"         // 업그레이드 레벨 조회
#include "DeliveryHUD.h"
#include "DeliveryTypes.h"            // EUpgradeType 열거형
#include "Kismet/GameplayStatics.h"   // SaveGameToSlot / LoadGameFromSlot
#include "GameFramework/Pawn.h"

// ── 생성자 ─────────────────────────────────────────────────────────────
AGamesetGameMode::AGamesetGameMode()
{
	// Tick 활성화 (배달 완료 폴링에 사용)
	PrimaryActorTick.bCanEverTick = true;

	DeliveryManager = CreateDefaultSubobject<UDeliveryManagerComponent>(TEXT("DeliveryManager"));
	HUDClass        = ADeliveryHUD::StaticClass();
	// DefaultPawnClass는 BP_DeliveryGameMode에서 샘플 캐릭터로 직접 지정
}

// ── BeginPlay ──────────────────────────────────────────────────────────
void AGamesetGameMode::BeginPlay()
{
	Super::BeginPlay();

	// ── 세이브 데이터 불러오기 (없으면 기본값 사용) ──────────────────
	LoadGame();

	// ── DeliveryComponent의 OnGameOver 델리게이트에 GameMode 핸들러 바인딩 ──
	// 배고픔 또는 갈증이 0이 될 때 DeliveryComponent가 OnGameOver를 브로드캐스트하면
	// GameMode의 TriggerGameOver가 호출된다.
	if (UDeliveryComponent* DC = GetDeliveryComponent())
	{
		// FOnGameOver 는 파라미터 없는 델리게이트이므로 람다로 이유 문자열을 추가
		DC->OnGameOver.AddDynamic(this, &AGamesetGameMode::OnDeliveryComponentGameOver);
	}
}

// ── Tick ───────────────────────────────────────────────────────────────
void AGamesetGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 게임이 이미 종료됐으면 폴링 불필요
	if (bGameEndTriggered) return;
	if (!DeliveryManager)  return;

	// DeliveryManager의 TotalDelivered가 증가했는지 폴링
	// → 증가한 경우 자동 저장 + 클리어 조건 체크
	const int32 Current = DeliveryManager->TotalDelivered;
	if (Current != LastKnownTotalDelivered)
	{
		LastKnownTotalDelivered = Current;

		// 배달 완료마다 자동 저장
		SaveGame();

		// 클리어 조건 검사
		CheckGameClearCondition();
	}
}

// ── TriggerGameOver ────────────────────────────────────────────────────
void AGamesetGameMode::TriggerGameOver(const FString& Reason)
{
	// 중복 호출 방지
	if (bGameEndTriggered) return;
	bGameEndTriggered = true;

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] GameOver! 원인: %s"), *Reason);

	// BP에서 UI 위젯을 띄우도록 델리게이트 브로드캐스트
	OnGameOver.Broadcast(Reason);

	// 게임 일시 정지 (선택적 — BP에서 SetPause 호출로 대체 가능)
	// APlayerController가 있을 때만 정지
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetPause(true);
	}
}

// ── TriggerGameClear ───────────────────────────────────────────────────
void AGamesetGameMode::TriggerGameClear()
{
	// 중복 호출 방지
	if (bGameEndTriggered) return;
	bGameEndTriggered = true;

	UE_LOG(LogTemp, Log, TEXT("[GameMode] GameClear! 배달 완료 횟수: %d"),
		DeliveryManager ? DeliveryManager->TotalDelivered : 0);

	// 클리어 시 최종 저장
	SaveGame();

	// BP에서 결과 화면을 띄우도록 델리게이트 브로드캐스트
	OnGameClear.Broadcast();
}

// ── SaveGame ───────────────────────────────────────────────────────────
void AGamesetGameMode::SaveGame()
{
	// 새 SaveGame 오브젝트 생성
	UGamesetSaveGame* SaveData = Cast<UGamesetSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UGamesetSaveGame::StaticClass()));
	if (!SaveData) return;

	// ── DeliveryComponent에서 데이터 읽기 ────────────────────────────
	if (UDeliveryComponent* DC = GetDeliveryComponent())
	{
		SaveData->SavedMoney   = DC->Money;
		SaveData->SavedGameDay = DC->GameDay;
		SaveData->SavedHunger  = DC->Hunger;
		SaveData->SavedThirst  = DC->Thirst;
	}

	// ── DeliveryManager에서 누적 배달 횟수 읽기 ──────────────────────
	if (DeliveryManager)
	{
		SaveData->SavedTotalDelivered = DeliveryManager->TotalDelivered;
	}

	// ── UpgradeComponent에서 각 업그레이드 레벨 읽기 ─────────────────
	if (UUpgradeComponent* UC = GetUpgradeComponent())
	{
		// EUpgradeType의 모든 값을 순회하여 레벨을 문자열 키로 저장
		const TMap<EUpgradeType, FUpgradeLevel>& AllUpgrades = UC->Upgrades;
		for (const TPair<EUpgradeType, FUpgradeLevel>& Pair : AllUpgrades)
		{
			// EUpgradeType → FString 변환 (StaticEnum 이용)
			const UEnum* EnumPtr = StaticEnum<EUpgradeType>();
			if (!EnumPtr) continue;
			FString EnumName = EnumPtr->GetNameStringByValue(static_cast<int64>(Pair.Key));
			SaveData->UpgradeLevels.Add(EnumName, Pair.Value.Level);
		}
	}

	// ── 슬롯에 저장 ──────────────────────────────────────────────────
	const bool bSuccess = UGameplayStatics::SaveGameToSlot(
		SaveData,
		UGamesetSaveGame::SaveSlotName,
		UGamesetSaveGame::UserIndex);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] SaveGame %s (슬롯: %s)"),
		bSuccess ? TEXT("성공") : TEXT("실패"),
		*UGamesetSaveGame::SaveSlotName);
}

// ── LoadGame ───────────────────────────────────────────────────────────
void AGamesetGameMode::LoadGame()
{
	// 슬롯에 저장된 데이터가 있는지 확인
	if (!UGameplayStatics::DoesSaveGameExist(
			UGamesetSaveGame::SaveSlotName,
			UGamesetSaveGame::UserIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] 저장 데이터 없음 — 기본값으로 시작"));
		return;
	}

	UGamesetSaveGame* SaveData = Cast<UGamesetSaveGame>(
		UGameplayStatics::LoadGameFromSlot(
			UGamesetSaveGame::SaveSlotName,
			UGamesetSaveGame::UserIndex));

	if (!SaveData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] LoadGame 실패 — 기본값으로 시작"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] LoadGame 성공 — Money=%.0f, Day=%d, Delivered=%d"),
		SaveData->SavedMoney, SaveData->SavedGameDay, SaveData->SavedTotalDelivered);

	// ── DeliveryComponent에 데이터 적용 ──────────────────────────────
	if (UDeliveryComponent* DC = GetDeliveryComponent())
	{
		DC->Money   = SaveData->SavedMoney;
		DC->GameDay = SaveData->SavedGameDay;
		DC->Hunger  = FMath::Clamp(SaveData->SavedHunger, 0.f, DC->HungerMax);
		DC->Thirst  = FMath::Clamp(SaveData->SavedThirst, 0.f, DC->ThirstMax);

		// Money 변경 델리게이트 브로드캐스트 (HUD 갱신)
		DC->OnMoneyChanged.Broadcast(DC->Money, 0.f);
	}

	// ── DeliveryManager에 누적 배달 횟수 적용 ────────────────────────
	if (DeliveryManager)
	{
		DeliveryManager->TotalDelivered = SaveData->SavedTotalDelivered;
		LastKnownTotalDelivered         = SaveData->SavedTotalDelivered;
	}

	// ── UpgradeComponent에 업그레이드 레벨 적용 ──────────────────────
	if (UUpgradeComponent* UC = GetUpgradeComponent())
	{
		const UEnum* EnumPtr = StaticEnum<EUpgradeType>();
		for (const TPair<FString, int32>& Pair : SaveData->UpgradeLevels)
		{
			if (!EnumPtr) break;
			const int64 EnumVal = EnumPtr->GetValueByNameString(Pair.Key);
			if (EnumVal == INDEX_NONE) continue;

			const EUpgradeType UpType = static_cast<EUpgradeType>(EnumVal);
			if (UC->Upgrades.Contains(UpType))
			{
				// 레벨을 직접 설정 (최댓값 초과 방지)
				FUpgradeLevel& UpLevel = UC->Upgrades[UpType];
				UpLevel.Level = FMath::Clamp(Pair.Value, 0, UpLevel.MaxLevel);
			}
		}
	}
}

// ── CheckGameClearCondition ────────────────────────────────────────────
void AGamesetGameMode::CheckGameClearCondition()
{
	if (!DeliveryManager) return;

	// 누적 배달 완료 횟수가 목표치 이상이면 클리어
	if (DeliveryManager->TotalDelivered >= GameClearDeliveryCount)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] 게임 클리어 조건 충족! (%d/%d)"),
			DeliveryManager->TotalDelivered, GameClearDeliveryCount);
		TriggerGameClear();
	}
}

// ── OnDeliveryComponentGameOver (내부 바인딩 핸들러) ──────────────────
// DeliveryComponent::OnGameOver (파라미터 없는 델리게이트)가 발동되면 호출됨.
// 어떤 스탯이 먼저 고갈됐는지 확인해 Reason 문자열을 만들어 TriggerGameOver 호출.
void AGamesetGameMode::OnDeliveryComponentGameOver()
{
	if (bGameEndTriggered) return;

	FString Reason = TEXT("생존 스탯 고갈");

	// 어떤 스탯이 고갈됐는지 판별해 더 구체적인 메시지 구성
	if (UDeliveryComponent* DC = GetDeliveryComponent())
	{
		if (DC->Hunger <= 0.f && DC->Thirst <= 0.f)
		{
			Reason = TEXT("배고픔과 갈증 동시 고갈");
		}
		else if (DC->Hunger <= 0.f)
		{
			Reason = TEXT("배고픔 고갈");
		}
		else if (DC->Thirst <= 0.f)
		{
			Reason = TEXT("갈증 고갈");
		}
	}

	TriggerGameOver(Reason);
}

// ── 유틸리티 ──────────────────────────────────────────────────────────
UDeliveryComponent* AGamesetGameMode::GetDeliveryComponent() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return nullptr;
	return PlayerPawn->FindComponentByClass<UDeliveryComponent>();
}

UUpgradeComponent* AGamesetGameMode::GetUpgradeComponent() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return nullptr;
	return PlayerPawn->FindComponentByClass<UUpgradeComponent>();
}
