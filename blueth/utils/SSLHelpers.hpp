#pragma once
#include <memory>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace blueth::detail {

struct WolfSSL_CTX_Deleter {
	void operator()(WOLFSSL_CTX *ctx) {
		::wolfSSL_CTX_free(ctx);
		::wolfSSL_Cleanup();
	}
};

struct WolfSSL_Deleter {
	void operator()(WOLFSSL *ssl) { ::wolfSSL_free(ssl); }
};

} // namespace blueth::detail
