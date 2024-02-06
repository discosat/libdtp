#include <assert.h>
#include <stdbool.h>

#include "cspftp/cspftp.h"

/**
 * @brief CSPFTP session, internal definition
 */
typedef struct cspftp_t
{
    cspftp_opt_remote_info remote_info;
    cspftp_errno_t errno;
} cspftp_t;


typedef struct {
    cspftp_t session;
    bool in_use;
} cspftp_static_session_t;

/**
 * @brief global CSPFTP error
 */
static cspftp_errno_t last_error = CSPFTP_NO_ERR;

static const char *const _error_strs[] = {
    "No error",
    "Invalid argument",
    "No more static sessions available",
    "Given session not found"
};

static_assert(CSPFTP_LAST_ERR <= sizeof(_error_strs) / sizeof(_error_strs[0]), "_error_strs does not have entries for all error codes");

static const uint32_t serializer_version = 1;

#define CSPFTP_NOF_STATIC_SESSIONS 5
static const uint8_t cspftp_nof_static_sessions = CSPFTP_NOF_STATIC_SESSIONS;
static cspftp_static_session_t static_sessions[CSPFTP_NOF_STATIC_SESSIONS] = {0};

cspftp_errno_t cspftp_errno(cspftp_t *session)
{
    if (0 != session) {
        return session->errno;
    }
    return last_error;
}

const char *cspftp_strerror(cspftp_errno_t err)
{
    return _error_strs[err];
}

cspftp_t *cspftp_acquire_session()
{
    for (uint8_t i = 0; i < cspftp_nof_static_sessions; i++) {
        if (false == static_sessions[i].in_use) {
            static_sessions[i].in_use = true;
            last_error = CSPFTP_NO_ERR;
            return &(static_sessions[i].session);
        }
    }
    last_error = CSPFTP_ENOMORE_SESSIONS;
    return 0;
}

cspftp_result cspftp_release_session(cspftp_t *session)
{
    for (uint8_t i = 0; i < cspftp_nof_static_sessions; i++) {
        if (session == &(static_sessions[i].session)) {
            static_sessions[i].in_use = false;
            last_error = CSPFTP_NO_ERR;
            return CSPFTP_OK;
        }
    }
    last_error = CSPFTP_ESESSION_NOT_FOUND;
    return CSPFTP_ERR;
}

cspftp_result cspftp_new_session(cspftp_t **cspftp_t)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_free_session(cspftp_t *session)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_set_opt(cspftp_t *session, cspftp_option option, cspftp_params *param)
{
    switch(option) {
        case CSPFTP_REMOTE_INFO: {
            session->remote_info.remote = param->remote_info.remote;
            break;
        }
        default:
            break;
    }
    return CSPFTP_OK;
}

cspftp_result cspftp_get_opt(cspftp_t *session, cspftp_option option, cspftp_params *param)
{
    switch(option) {
        case CSPFTP_REMOTE_INFO: {
            param->remote_info.remote = session->remote_info.remote;
            break;
        }
        default:
            break;
    }
    return CSPFTP_OK;
}

cspftp_result cspftp_start_transfer(cspftp_t *session)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_stop_transfer(cspftp_t *session)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_set_source(cspftp_t *session, vmem_t *src)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_set_destination(cspftp_t *session, vmem_t *dest)
{
    return CSPFTP_OK;
}

cspftp_result cspftp_serialize_session(cspftp_t *session, vmem_t *dest)
{
    uint32_t offset = 0;
    /* Write serialization version number first */
    dest->write(dest, offset, &serializer_version, sizeof(serializer_version));
    offset += sizeof(serializer_version);
    return CSPFTP_OK;
}

cspftp_result cspftp_deserialize_session(cspftp_t *session, vmem_t *src) {
    return CSPFTP_OK;
}