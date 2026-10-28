#pragma once

#define aidl_error(e) (-(e))

#define DL_OK 1
#define DL_IGNORE 1

#define DL_ERR_FAILED                     aidl_error(1)
#define DL_ERR_PARAMETER_NULL             aidl_error(2)
#define DL_ERR_CLOSED                     aidl_error(3)
#define DL_ERR_WANT_WRITE                 aidl_error(4)
#define DL_ERR_WANT_READ                  aidl_error(5)
#define DL_ERR_FRAME_MALFORMED            aidl_error(6)
#define DL_ERR_FRAME_DERAILED             aidl_error(7)
#define DL_ERR_TARGET_TOO_SMALL           aidl_error(8)
#define DL_ERR_TARGET_NOT_FOUND           aidl_error(9)