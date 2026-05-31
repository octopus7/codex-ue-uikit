#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CodexUIKitWBPBakeCommandlet.generated.h"

UCLASS()
class UCodexUIKitWBPBakeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UCodexUIKitWBPBakeCommandlet();

	virtual int32 Main(const FString& Params) override;
};
