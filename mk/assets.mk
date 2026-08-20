# ============================================================================
#  Format-aware asset validation
# ============================================================================

PSX_TIM_ASSETS := $(call rwildcard,assets,*.tim)

.PHONY: validate-assets validate-psx-tim-assets

validate-assets: validate-psx-tim-assets

validate-psx-tim-assets:
ifneq ($(strip $(PSX_TIM_ASSETS)),)
	python3 tools/assets/psx_tim.py roundtrip $(PSX_TIM_ASSETS)
else
	@:
endif
