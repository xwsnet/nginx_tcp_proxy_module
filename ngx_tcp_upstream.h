
#ifndef _NGX_TCP_UPSTREAM_H_INCLUDED_
#define _NGX_TCP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_tcp.h>


#define NGX_TCP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_TCP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_TCP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_TCP_UPSTREAM_FT_TCP_500         0x00000010
#define NGX_TCP_UPSTREAM_FT_TCP_502         0x00000020
#define NGX_TCP_UPSTREAM_FT_TCP_503         0x00000040
#define NGX_TCP_UPSTREAM_FT_TCP_504         0x00000080
#define NGX_TCP_UPSTREAM_FT_TCP_404         0x00000100
#define NGX_TCP_UPSTREAM_FT_UPDATING        0x00000200
#define NGX_TCP_UPSTREAM_FT_BUSY_LOCK       0x00000400
#define NGX_TCP_UPSTREAM_FT_MAX_WAITING     0x00000800
#define NGX_TCP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_TCP_UPSTREAM_FT_OFF             0x80000000

#define NGX_TCP_UPSTREAM_FT_STATUS          (NGX_TCP_UPSTREAM_FT_TCP_500  \
                                             |NGX_TCP_UPSTREAM_FT_TCP_502  \
                                             |NGX_TCP_UPSTREAM_FT_TCP_503  \
                                             |NGX_TCP_UPSTREAM_FT_TCP_504  \
                                             |NGX_TCP_UPSTREAM_FT_TCP_404)

#define NGX_TCP_UPSTREAM_INVALID_HEADER     40


#define NGX_TCP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_TCP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_TCP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_TCP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010

typedef struct ngx_tcp_upstream_srv_conf_s  ngx_tcp_upstream_srv_conf_t;


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    ngx_uint_t                       status;
    time_t                           response_sec;
    ngx_uint_t                       response_msec;
    off_t                           response_length;

    ngx_str_t                       *peer;
} ngx_tcp_upstream_state_t;

typedef struct {
    u_char major;
    u_char minor;
} ssl_protocol_version_t;

typedef struct {
    u_char                 msg_type;
    ssl_protocol_version_t version;
    uint16_t               length;

    u_char                 handshake_type;
    u_char                 handshake_length[3];
    ssl_protocol_version_t hello_version;

    time_t                 time;
    u_char                 random[28];

    u_char                 others[0];
} __attribute__((packed)) server_ssl_hello_t;

typedef struct {
    ngx_buf_t send;
    ngx_buf_t recv;
} ngx_tcp_check_ssl_hello_ctx;


/*state*/
#define NGX_TCP_CHECK_CONNECT_DONE     0x0001
#define NGX_TCP_CHECK_SEND_DONE        0x0002
#define NGX_TCP_CHECK_RECV_DONE        0x0004
#define NGX_TCP_CHECK_ALL_DONE         0x0008

typedef struct {
    ngx_pid_t  owner;

    ngx_msec_t access_time;

    ngx_uint_t fall_count;
    ngx_uint_t rise_count;

    ngx_atomic_t lock;
    ngx_atomic_t busy;
    ngx_atomic_t down;

    unsigned last_down:1;
} ngx_tcp_check_peer_shm_t;

typedef struct {
    ngx_uint_t generation;

    ngx_uint_t state;
    ngx_atomic_t lock;

    /*store the ngx_tcp_check_status_peer_t*/
    ngx_tcp_check_peer_shm_t peers[0];
} ngx_tcp_check_peers_shm_t;

typedef struct {
    ngx_flag_t                       state;
    ngx_pool_t                      *pool;
    ngx_uint_t                       index;
    ngx_tcp_upstream_srv_conf_t     *conf;
    ngx_peer_addr_t                 *peer;
    ngx_event_t                      check_ev;
    ngx_event_t                      check_timeout_ev;
    ngx_peer_connection_t            pc;
    void *                           check_data;
    ngx_tcp_check_peer_shm_t         *shm;
} ngx_tcp_check_peer_conf_t;

typedef struct {
    ngx_str_t                        check_shm_name;
    ngx_array_t                      peers;

    ngx_tcp_check_peers_shm_t       *peers_shm;
} ngx_tcp_check_peers_conf_t;

typedef struct {
    ngx_uint_t                       check_shm_size;
    ngx_tcp_check_peers_conf_t       peers_conf;
    ngx_array_t                      upstreams; /* ngx_tcp_upstream_srv_conf_t */
} ngx_tcp_upstream_main_conf_t;


typedef ngx_int_t (*ngx_tcp_upstream_init_pt)(ngx_conf_t *cf,
        ngx_tcp_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_tcp_upstream_init_peer_pt)(ngx_tcp_session_t *s,
        ngx_tcp_upstream_srv_conf_t *us);

typedef struct {
    ngx_tcp_upstream_init_pt        init_upstream;
    ngx_tcp_upstream_init_peer_pt   init;
    void                           *data;
} ngx_tcp_upstream_peer_t;

typedef struct {
    ngx_peer_addr_t                 *addrs;
    ngx_uint_t                       naddrs;
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;
    time_t                           fail_timeout;
    ngx_uint_t                       max_busy;
    ngx_str_t                        srun_id;

    unsigned                         down:1;
    unsigned                         backup:1;
} ngx_tcp_upstream_server_t;


#define NGX_TCP_UPSTREAM_CREATE        0x0001
#define NGX_TCP_UPSTREAM_WEIGHT        0x0002
#define NGX_TCP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_TCP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_TCP_UPSTREAM_DOWN          0x0010
#define NGX_TCP_UPSTREAM_BACKUP        0x0020
#define NGX_TCP_UPSTREAM_SRUN_ID       0x0040
#define NGX_TCP_UPSTREAM_MAX_BUSY      0x0080


/*TODO: add more check method*/
#define NGX_TCP_CHECK_TCP              0x0001
#define NGX_TCP_CHECK_HTTP             0x0002
#define NGX_TCP_CHECK_SSL_HELLO        0x0004
#define NGX_TCP_CHECK_SMTP             0x0008

struct ngx_tcp_upstream_srv_conf_s {
    ngx_tcp_upstream_peer_t          peer;
    void                           **srv_conf;

    ngx_array_t                     *servers;  /* ngx_tcp_upstream_server_t */

    ngx_uint_t                       flags;
    ngx_str_t                        host;
    u_char                          *file_name;
    ngx_uint_t                       line;
    in_port_t                        port;
    in_port_t                        default_port;


    ngx_uint_t                       fall_count;
    ngx_uint_t                       rise_count;
    ngx_msec_t                       check_interval;
    ngx_msec_t                       check_timeout;
    ngx_uint_t                       check_type;
};


typedef struct {
    ngx_tcp_upstream_srv_conf_t    *upstream;

    ngx_msec_t                       connect_timeout;
    ngx_msec_t                       send_timeout;
    ngx_msec_t                       read_timeout;
    ngx_msec_t                       timeout;

    /*size_t                           send_lowat;*/
    /*size_t                           buffer_size;*/

    /*size_t                           busy_buffers_size;*/
    /*size_t                           max_temp_file_size;*/
    /*size_t                           temp_file_write_size;*/

    /*size_t                           busy_buffers_size_conf;*/
    /*size_t                           max_temp_file_size_conf;*/
    /*size_t                           temp_file_write_size_conf;*/

    /*ngx_bufs_t                       bufs;*/

    /*ngx_uint_t                       ignore_headers;*/
    /*ngx_uint_t                       next_upstream;*/
    /*ngx_uint_t                       store_access;*/
    /*ngx_flag_t                       buffering;*/
    /*ngx_flag_t                       pass_request_headers;*/
    /*ngx_flag_t                       pass_request_body;*/

    /*ngx_flag_t                       ignore_client_abort;*/
    /*ngx_flag_t                       intercept_errors;*/
    /*ngx_flag_t                       cyclic_temp_file;*/

    /*ngx_path_t                      *temp_path;*/

    /*ngx_hash_t                       hide_headers_hash;*/
    /*ngx_array_t                     *hide_headers;*/
    /*ngx_array_t                     *pass_headers;*/

    /*#if (NGX_TCP_CACHE)*/
    /*ngx_shm_zone_t                  *cache;*/

    /*ngx_uint_t                       cache_min_uses;*/
    /*ngx_uint_t                       cache_use_stale;*/
    /*ngx_uint_t                       cache_methods;*/

    /*ngx_array_t                     *cache_valid;*/
    /*#endif*/

    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;

    /*signed                           store:2;*/
    /*unsigned                         intercept_404:1;*/
    /*unsigned                         change_buffering:1;*/

    /*#if (NGX_TCP_SSL)*/
    /*ngx_ssl_t                       *ssl;*/
    /*ngx_flag_t                       ssl_session_reuse;*/
    /*#endif*/

} ngx_tcp_upstream_conf_t;


/*typedef struct {*/
/*ngx_str_t                        name;*/
/**//*ngx_tcp_header_handler_pt       handler;*/
/*ngx_uint_t                       offset;*/
/**//*ngx_tcp_header_handler_pt       copy_handler;*/
/*ngx_uint_t                       conf;*/
/*ngx_uint_t                       redirect;  *//* unsigned   redirect:1; */
/*} ngx_tcp_upstream_header_t;*/


/*typedef struct {*/
    /*ngx_list_t                       headers;*/

/*ngx_uint_t                       status_n;*/
/*ngx_str_t                        status_line;*/

/*ngx_table_elt_t                 *status;*/
/*ngx_table_elt_t                 *date;*/
/*ngx_table_elt_t                 *server;*/
/*ngx_table_elt_t                 *connection;*/

/*ngx_table_elt_t                 *expires;*/
/*ngx_table_elt_t                 *etag;*/
/*ngx_table_elt_t                 *x_accel_expires;*/
/*ngx_table_elt_t                 *x_accel_redirect;*/
/*ngx_table_elt_t                 *x_accel_limit_rate;*/

/*ngx_table_elt_t                 *content_type;*/
/*ngx_table_elt_t                 *content_length;*/

/*ngx_table_elt_t                 *last_modified;*/
/*ngx_table_elt_t                 *location;*/
/*ngx_table_elt_t                 *accept_ranges;*/
/*ngx_table_elt_t                 *www_authenticate;*/

/*#if (NGX_TCP_GZIP)*/
/*ngx_table_elt_t                 *content_encoding;*/
/*#endif*/

/*off_t                            content_length_n;*/

/*ngx_array_t                      cache_control;*/
/*} ngx_tcp_upstream_headers_in_t;*/


typedef struct {
    ngx_str_t                        host;
    in_port_t                        port;
    ngx_uint_t                       no_port; /* unsigned no_port:1 */

    ngx_uint_t                       naddrs;
    in_addr_t                       *addrs;

    struct sockaddr                 *sockaddr;
    socklen_t                        socklen;

    ngx_resolver_ctx_t              *ctx;
} ngx_tcp_upstream_resolved_t;


typedef void (*ngx_tcp_upstream_handler_pt)(ngx_tcp_session_t *s,
        ngx_tcp_upstream_t *u);

struct ngx_tcp_upstream_s {
    ngx_tcp_upstream_handler_pt     read_event_handler;
    ngx_tcp_upstream_handler_pt     write_event_handler;

    ngx_peer_connection_t            peer;

    ngx_event_pipe_t                *pipe;

    /*ngx_chain_t                     *request_bufs;*/

    /*ngx_output_chain_ctx_t           output;*/
    /*ngx_chain_writer_ctx_t           writer;*/

    ngx_tcp_upstream_conf_t        *conf;

    /*ngx_tcp_upstream_headers_in_t   headers_in;*/

    ngx_tcp_upstream_resolved_t    *resolved;

    /*ngx_buf_t                        buffer;*/
    /*size_t                           length;*/

    /*ngx_chain_t                     *out_bufs;*/
    /*ngx_chain_t                     *busy_bufs;*/
    /*ngx_chain_t                     *free_bufs;*/

    ngx_int_t                      (*input_filter_init)(void *data);
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes);
    void                            *input_filter_ctx;

#if (NGX_TCP_CACHE)
    ngx_int_t                      (*create_key)(ngx_tcp_session_t *r);
#endif
    ngx_int_t                      (*create_request)(ngx_tcp_session_t *r);
    ngx_int_t                      (*reinit_request)(ngx_tcp_session_t *r);
    ngx_int_t                      (*process_header)(ngx_tcp_session_t *r);
    void                           (*abort_request)(ngx_tcp_session_t *r);
    void                           (*finalize_request)(ngx_tcp_session_t *r, ngx_int_t rc);
    ngx_int_t                      (*rewrite_redirect)(ngx_tcp_session_t *r, ngx_table_elt_t *h, size_t prefix);

    /*ngx_msec_t                       timeout;*/

    ngx_tcp_upstream_state_t       *state;

    /*ngx_str_t                        method;*/
    /*ngx_str_t                        schema;*/
    /*ngx_str_t                        uri;*/

    ngx_tcp_cleanup_pt             *cleanup;

    /*unsigned                         store:1;*/
    /*unsigned                         cacheable:1;*/
    /*unsigned                         accel:1;*/
    /*unsigned                         ssl:1;*/
    /*#if (NGX_TCP_CACHE)*/
    /*unsigned                         cache_status:3;*/
    /*#endif*/

    /*unsigned                         buffering:1;*/

    /*unsigned                         request_sent:1;*/
    /*unsigned                         header_sent:1;*/
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_tcp_upstream_next_t;


ngx_int_t ngx_tcp_upstream_create(ngx_tcp_session_t *s);
void ngx_tcp_upstream_init(ngx_tcp_session_t *s);
ngx_tcp_upstream_srv_conf_t *ngx_tcp_upstream_add(ngx_conf_t *cf,
        ngx_url_t *u, ngx_uint_t flags);
void ngx_tcp_upstream_proxy_generic_handler(ngx_tcp_session_t *s, ngx_tcp_upstream_t *u);

ngx_int_t ngx_tcp_upstream_check_broken_connection(ngx_tcp_session_t *s);
void ngx_tcp_upstream_next(ngx_tcp_session_t *s, ngx_tcp_upstream_t *u, ngx_uint_t ft_type);

ngx_uint_t ngx_tcp_check_add_peer(ngx_conf_t *cf, ngx_tcp_upstream_srv_conf_t *uscf,
        ngx_peer_addr_t *peer);

ngx_uint_t ngx_tcp_check_peer_down(ngx_uint_t index);

#define ngx_tcp_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_tcp_upstream_module;


#endif /* _NGX_TCP_UPSTREAM_H_INCLUDED_ */
