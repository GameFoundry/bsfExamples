set(BS_COMMON_INC_NOFILTER
	"BsExampleFramework.h"
	"BsCameraFlyer.h"
	"BsObjectRotator.h"
	"BsFPSWalker.h"
	"BsFPSCamera.h"
)

set(BS_COMMON_SRC_NOFILTER
	"BsCameraFlyer.cpp"
	"BsObjectRotator.cpp"
	"BsFPSWalker.cpp"
	"BsFPSCamera.cpp"
)

set(BS_COMMON_SRC
	${BS_COMMON_INC_NOFILTER}
	${BS_COMMON_SRC_NOFILTER}
)