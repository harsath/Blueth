#pragma once
#include "HTTPConstants.hpp"
#include "HTTPMessage.hpp"
#include "utils/UnixTime.hpp"
#include <memory>
#include <string>

namespace blueth::http {

// We will add more, if needed
enum class TemplateType {
	Ok,
	BadRequest,
	NotFound,
	Forbidden,
	NotAcceptable,
	MethodNotAllowed,
	UnsupportedMediaType,
	Created,
	MovedPermanently,
	Unauthorized
};

inline std::unique_ptr<HTTPResponseMessage>
HTTPResponseTemplates(const TemplateType &type, std::string http_message_body) {
	std::unique_ptr<HTTPResponseMessage> message =
	    HTTPResponseMessage::create();
	switch (type) {
	case TemplateType::Ok:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::Ok);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::BadRequest:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::BadRequest);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::UnsupportedMediaType:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(
		    HTTPResponseCodes::UnsupportedMediaType);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::Created:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::Created);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::Forbidden:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::Forbidden);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::MethodNotAllowed:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::MethodNotAllowed);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::MovedPermanently:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::MovedPermanently);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::NotFound:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::NotFound);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	case TemplateType::Unauthorized:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::BadRequest);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->addHeader("WWW-Authenticate",
				   std::move(http_message_body));
		break;
	case TemplateType::NotAcceptable:
		message->setHTTPVersion(HTTPVersion::HTTP1_1);
		message->setResponseCode(HTTPResponseCodes::NotAcceptable);
		message->addHeader("Content-Type", "text/html");
		message->addHeader("Content-Length",
				   std::to_string(http_message_body.size()));
		message->addHeader("Date", UnixTime{}.getStringTime());
		message->pushBackRawBody(std::move(http_message_body));
		break;
	}
	return message;
}

} // namespace blueth::http
