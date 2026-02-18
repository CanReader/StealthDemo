// © 2025 Canberk. All Rights Reserved.

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
class CANBERKAIDEMO_API UAlertWidget : public UUserWidget
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

            // Dynamic color gradient: green → yellow → orange → red
            FLinearColor BarColor;
            if (Value < 0.4f)
            {
                BarColor = FMath::Lerp(FLinearColor::Green, FLinearColor::Yellow, Value / 0.4f);
            }
            else if (Value < 0.7f)
            {
                FLinearColor Orange(1.0f, 0.5f, 0.0f);
                BarColor = FMath::Lerp(FLinearColor::Yellow, Orange, (Value - 0.4f) / 0.3f);
            }
            else
            {
                FLinearColor Orange(1.0f, 0.5f, 0.0f);
                BarColor = FMath::Lerp(Orange, FLinearColor::Red, (Value - 0.7f) / 0.3f);
            }
            ProgressBar->SetFillColorAndOpacity(BarColor);
        }
    }
};
