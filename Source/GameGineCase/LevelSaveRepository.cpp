#include "LevelSaveRepository.h"
#include "LevelSave.h"
#include "Kismet/GameplayStatics.h"

void ULevelSaveRepository::SaveGame(bool bLevel1Completed, bool bLevel2Completed, bool bLevel3Completed)
{

    ULevelSave* Saver = Cast<ULevelSave>(UGameplayStatics::CreateSaveGameObject(ULevelSave::StaticClass()));
    
    Saver->bLevel1Completed = bLevel1Completed;
    Saver->bLevel2Completed = bLevel2Completed;
    Saver->bLevel3Completed = bLevel3Completed;

    if (UGameplayStatics::SaveGameToSlot(Saver, "CompletedLevels", 0))
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1,15,FColor::Red,TEXT("Failed to save levels!!"));
    }

}

void ULevelSaveRepository::LoadGame(bool& bLevel1Completed, bool& bLevel2Completed, bool& bLevel3Completed)
{
    if (UGameplayStatics::DoesSaveGameExist("CompletedLevels", 0))
    {
        ULevelSave* Loader = Cast<ULevelSave>(UGameplayStatics::LoadGameFromSlot("CompletedLevels", 0));

        bLevel1Completed = Loader->bLevel1Completed;
        bLevel2Completed = Loader->bLevel2Completed;
        bLevel3Completed = Loader->bLevel3Completed;
    }
    else
    {
        // If there's no save file, create a new one with default values
        ULevelSave* Saver = Cast<ULevelSave>(UGameplayStatics::CreateSaveGameObject(ULevelSave::StaticClass()));

        UGameplayStatics::SaveGameToSlot(Saver, "CompletedLevels", 0);

        bLevel1Completed = Saver->bLevel1Completed;
        bLevel2Completed = Saver->bLevel2Completed;
        bLevel3Completed = Saver->bLevel3Completed;
    }
}
