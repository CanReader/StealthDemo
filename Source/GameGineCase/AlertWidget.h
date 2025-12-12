// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/ProgressBar.h>
#include <Components/Image.h>
#include "AlertWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API UAlertWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidgetOptional))
    UProgressBar* ProgressBar;

    // if you want an arrow / icon:
    UPROPERTY(meta=(BindWidgetOptional))
    UImage* IconImage;

    UFUNCTION(BlueprintCallable)
    void SetNoticeProgress(float Value)
    {
        if (ProgressBar)
        {
            ProgressBar->SetPercent(Value);
        }
    }	
};
