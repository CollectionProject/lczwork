#ifndef _HTTP_CLI_H_
#define _HTTP_CLI_H_


/*! \addtogroup HTTP_CLIENT
 *  @ingroup NETWORK_LIB
 *  @brief	HTTP/HTTPS client library api
 *  @{
 */



/**
 * @brief HTTP/HTTPS client function return error code
 */
typedef enum {
    HERROR_REDIRECT		        = 1,		/*!< not used   */
    HERROR_OK			        = 0,
    HERROR_MEM			        = -1,		/*!< memory malloc fail   */
    HERROR_HEADER		        = -2,		/*!< set header from url fail   */
    HERROR_RESPOND		        = -3,		/*!< http server respond error   */
    HERROR_SOCK			        = -4,		/*!< socket connect fail  */
    HERROR_CALLBACK		        = -5,		/*!< user callback function return not zero  */
    HERROR_UNKNOWN		        = -6,		/*!< socket receive process is interrupted   */
    HERROR_PARAM		        = -7,		/*!< input parameter error   */
    HERROR_REDIRECT_DEEP		= -8,		/*!< max redirect deep only support 4    */
    HERROR_BODY_ANALYSIS        = -9,		/*!< When user cb function is NULL, will use default cb function, transfer_encoding:chunked receive fail wille return this value   */
    HERROR_SOCKHDL              = -10,
} httpin_error;

/**
 * @brief HTTP/HTTPS client process status
 * @note This is used in user callback function
 */
typedef enum {
    HTTPIN_HEADER,
    HTTPIN_PROGRESS,
    HTTPIN_FINISHED,
    HTTPIN_ABORT,
    HTTPIN_ERROR,
    HTTPIN_NON_BLOCK,
} httpin_status;

/****************************************************/

typedef int (*httpcli_cb)(void *httpcli_ctx, void *buf, unsigned int size, void *priv, httpin_status status);

/**
 * @brief The header of buffer which save http body content
 * @note When memory not enough, the pointer p will auto realloc, and buf_count auto +1
 */
typedef struct _http_body_obj {
    char *p;		/*!< user malloc memory pointer   */
    unsigned int buf_len;		/*!< The size of one receive buffer   */
    unsigned int recv_len;		/*!< The content bytes had received   */
    unsigned char buf_count;		/*!< The count of receive buffer   */
} http_body_obj;

typedef struct _http_data_box {
    int  len;
    int  rptr;
    char buf[512];
} http_data_box;

#define HTTP_POST_MORE_DATA
#define HTTP_USE_DATA_BOX
/**
 * @brief HTTP/HTTPS client context structure
 */
typedef struct httpcli_ctx {
    void *sock_hdl;
    //user need init
    const char *url;		/*!< url   */
    char *redirection_url;		/*!< redirection_url   */
    const char *user_http_header; 		/*!< user parse http header in person, must finish in '\0'   */
    unsigned int lowRange;		/*!< Start byte range of the content to get    */
    unsigned int highRange;		/*!< Finish byte range of the content to get  */
    httpcli_cb cb;		/*!< user callback function   */
    void *priv;		/*!< The pointer to the private data of the callback function   */
    const char *post_data;		/*!< The pointer to the post data   */
    char *data_format;		/*!< The format type of the post data   */
    int data_len;		/*!< The size of the post data   */
    int timeout_millsec;		/*!< The timeout value of the connect, send, receive   */
    char *connection;		/*!< "close" or "keep-alive"    */
    //user get
    int content_length;		/*!< The length of the content get from the server   */
    char content_type[64];		/*!< The type of the content get from the server   */
    char transfer_encoding[64];		/*!< The transfer encoding method of the content  */

    //priv
    int exit_flag;
    int req_exit_flag;

    int chunked_last_read;
    char chunked_header_find[10];
    unsigned char chunked_header_offset;
    unsigned char support_range;
#ifdef HTTP_POST_MORE_DATA
    int *more_data;
    int *more_data_len;
#endif

#ifdef HTTP_USE_DATA_BOX
    http_data_box box;
#endif
} httpcli_ctx;

struct net_download_ops {
    httpin_error(*init)(httpcli_ctx *ctx);
    void (*close)(httpcli_ctx *ctx);
    void (*quit)(httpcli_ctx *ctx);
    int (*read)(httpcli_ctx *ctx, char *recvbuf, unsigned int len);
};

/****************************************************/

/**
 * @brief Http client get
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 *
 * @return httpin_error
 */
httpin_error httpcli_get(httpcli_ctx *ctx);

/**
 * @brief Http client post
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 *
 * @return httpin_error
 */
httpin_error httpcli_post(httpcli_ctx *ctx);

/**
 * @brief Break up the http client process
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 */
void httpcli_cancel(httpcli_ctx *ctx);

/**
 * @brief Http client get on SSL
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 *
 * @return httpin_error
 */
httpin_error httpscli_get(httpcli_ctx *ctx);

/**
 * @brief Http client post on SSL
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 *
 * @return httpin_error
 */
httpin_error httpscli_post(httpcli_ctx *ctx);

/**
 * @brief Break up the http client process on SSL
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 */
void httpscli_cancel(httpcli_ctx *ctx);

/**
 * @brief Get the length of respond http body content
 *
 * @param ctx The pointer to HTTP/HTTPS client context structure
 *
 * @return The content length
 */
int httpcli_content_length(httpcli_ctx *ctx);

httpin_error httpcli_post_header(httpcli_ctx *ctx);

httpin_error httpcli_chunked_send(httpcli_ctx *ctx);

extern const struct net_download_ops http_ops;

extern const struct net_download_ops https_ops;

int transfer_chunked_parse(httpcli_ctx *ctx, char *receive_buf, int recv_length, unsigned int (*cb)(unsigned char *, int, void *), void *priv);

int transfer_chunked_analysis(char *receive_buf, unsigned int recv_length);

/*! @}*/


#endif  //_HTTP_CLI_H_
