#include "request.hpp"
#include "main.hpp"
#include <map>
#include <netinet/in.h>

Request::Request(const pollfd& pfd) : _fd(pfd.fd) {
	if (pfd.revents != POLLIN)
		exit_with::message("Unexpected revents value");
	_read_request(pfd);
}

Request::~Request() {
}

void Request::_read_request(const pollfd& pfd) {
	static char buf[BUFFER_SIZE + 1];
	ssize_t		bytes_read;
	do {
		bytes_read = read(pfd.fd, buf, BUFFER_SIZE);
		if (bytes_read == -1)
			exit_with::e_perror("Unable to read from file");
		if (bytes_read == 0)
			break;
		buf[bytes_read] = '\0';
		this->raw += buf;
	} while (!_is_end_of_http_request(this->raw));
	_parse_request();
}

bool Request::_is_end_of_http_request(const std::string& s) {
	if (s.find("\r\n\r\n") == std::string::npos)
		return 0;
	return 1;
}

// Request-Line = Method SP Request-URI SP HTTP-Version CRLF
void Request::_parse_request_line() { // TO DO: check method (1/3) and http version(1/0/1.1)
	size_t		end;
	size_t		sp;
	std::string keys[] = {"method", "URI", "HTTP_version"};

	end = raw.find("\r\n");
	if (end == std::string::npos)
		exit_with::message("[CRLF]invalid request-line: 400 bad request/301 moved permanently");
	std::string line = raw.substr(0, end);

	sp = line.find_first_of(' ');
	if (sp == std::string::npos)
		exit_with::message("[method]invalid request-line: 400 bad request/301 moved permanently");
	_request_line["method"] = line.substr(0, sp);
	line.erase(0, sp + 1);

	sp = line.find_first_of(' ');
	if (sp == std::string::npos)
		exit_with::message("[URI]invalid request-line: 400 bad request/301 moved permanently");
	_request_line["URI"] = line.substr(0, sp);
	line.erase(0, sp + 1);

	sp = line.find_first_of(' ');
	if (sp != std::string::npos)
		exit_with::message("[HTTP_versison]invalid request-line: 400 bad request/301 moved permanently");
	_request_line["HTTP_version"] = line;
}

void Request::_parse_request() {
	_parse_request_line();
}

std::map<std::string, std::string> Request::get_request_line() const {
	return _request_line;
}

std::map<std::string, std::string> Request::get_request_headers() const {
	return _request_headers;
}

std::map<std::string, std::string> Request::get_body() const {
	return _body;
}

std::ostream& operator<<(std::ostream& output, Request const& rhs) {
	std::map<std::string, std::string> request_line = rhs.get_request_line();
	std::map<std::string, std::string> request_headers = rhs.get_request_headers();
	std::map<std::string, std::string> body = rhs.get_body();

	output << "Request line-------------" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request_line.begin(); it != request_line.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	output << "Request header fields----" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request_headers.begin(); it != request_headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	output << "Request body-------------" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = body.begin(); it != body.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	return output;
}

// TODO: optimize
void Request::send_response(uint32_t response_code, const std::string& message) {
	std::map<uint32_t, std::string> m; // TODO: make this static
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[204] = "No Content";
	m[301] = "Moved Permanently";
	m[302] = "Moved Temporarily";
	m[304] = "Not Modified";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";

	std::string response = "HTTP/1.1 ";
	response += cpp11::to_string(response_code);
	response += " ";
	response += m[response_code];
	response += "\r\n\r\n";
	response += message;
	write(_fd, response.c_str(), response.length()); // TODO: error handling
}
