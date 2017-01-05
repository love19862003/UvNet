/********************************************************************
    Filename: NetHttp.cpp
    Description:
    Version:  1.0
    Created:  20:12:2016   16:00

    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#include <sstream>
#include "net/NetHttp.h"
#include "net/http_parser.h"
#include "log/MyLog.h"
namespace ShareSpace{
  namespace NetSpace{
    
    namespace stock_replies{

      const char ok[] = "";
      const char created[] =
        "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
      const char accepted[] =
        "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
      const char no_content[] =
        "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
      const char multiple_choices[] =
        "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
      const char moved_permanently[] =
        "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
      const char moved_temporarily[] =
        "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
      const char not_modified[] =
        "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
      const char bad_request[] =
        "<html>"
        "<head><title>Bad Request</title></head>"
        "<body><h1>400 Bad Request</h1></body>"
        "</html>";
      const char unauthorized[] =
        "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
      const char forbidden[] =
        "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
      const char not_found[] =
        "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
      const char internal_server_error[] =
        "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
      const char not_implemented[] =
        "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
      const char bad_gateway[] =
        "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
      const char service_unavailable[] =
        "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";

#define HTTPSTATE(name)HTTP_STATUS_##name
      std::string to_string(int32 status){
        switch(status){
        case HTTPSTATE(OK):
          return ok;
        case HTTPSTATE(CREATED):
          return created;
        case HTTPSTATE(ACCEPTED):
          return accepted;
        case HTTPSTATE(NO_CONTENT):
          return no_content;
        case HTTPSTATE(MULTIPLE_CHOICES):
          return multiple_choices;
        case HTTPSTATE(MOVED_PERMANENTLY):
          return moved_permanently;
        case HTTPSTATE(FOUND):
          return moved_temporarily;
        case HTTPSTATE(NOT_MODIFIED):
          return not_modified;
        case HTTPSTATE(BAD_REQUEST):
          return bad_request;
        case HTTPSTATE(UNAUTHORIZED):
          return unauthorized;
        case HTTPSTATE(FORBIDDEN):
          return forbidden;
        case HTTPSTATE(NOT_FOUND):
          return not_found;
        case HTTPSTATE(INTERNAL_SERVER_ERROR):
          return internal_server_error;
        case HTTPSTATE(NOT_IMPLEMENTED):
          return not_implemented;
        case HTTPSTATE(BAD_GATEWAY):
          return bad_gateway;
        case HTTPSTATE(SERVICE_UNAVAILABLE):
          return service_unavailable;
        default:
          return internal_server_error;
        }
      }
#undef HTTPSTATE
    } // namespace stock_replies

    namespace status_strings{

     static const std::string ok =
        "HTTP/1.0 200 OK\r\n";
     static const std::string created =
        "HTTP/1.0 201 Created\r\n";
     static const std::string accepted =
        "HTTP/1.0 202 Accepted\r\n";
     static const std::string no_content =
        "HTTP/1.0 204 No Content\r\n";
     static const std::string multiple_choices =
        "HTTP/1.0 300 Multiple Choices\r\n";
     static const std::string moved_permanently =
        "HTTP/1.0 301 Moved Permanently\r\n";
     static const std::string moved_temporarily =
        "HTTP/1.0 302 Moved Temporarily\r\n";
     static const std::string not_modified =
        "HTTP/1.0 304 Not Modified\r\n";
     static const std::string bad_request =
        "HTTP/1.0 400 Bad Request\r\n";
     static const std::string unauthorized =
        "HTTP/1.0 401 Unauthorized\r\n";
     static const std::string forbidden =
        "HTTP/1.0 403 Forbidden\r\n";
     static const std::string not_found =
        "HTTP/1.0 404 Not Found\r\n";
     static const std::string internal_server_error =
        "HTTP/1.0 500 Internal Server Error\r\n";
     static const std::string not_implemented =
        "HTTP/1.0 501 Not Implemented\r\n";
     static const std::string bad_gateway =
        "HTTP/1.0 502 Bad Gateway\r\n";
     static const std::string service_unavailable =
        "HTTP/1.0 503 Service Unavailable\r\n";

#define HTTPSTATE(name)HTTP_STATUS_##name
      const std::string& to_string(int32 status){
        switch(status){
        case HTTPSTATE(OK):
          return ok;
        case HTTPSTATE(CREATED):
          return created;
        case HTTPSTATE(ACCEPTED):
          return accepted;
        case HTTPSTATE(NO_CONTENT):
          return no_content;
        case HTTPSTATE(MULTIPLE_CHOICES):
          return multiple_choices;
        case HTTPSTATE(MOVED_PERMANENTLY):
          return moved_permanently;
        case HTTPSTATE(FOUND):
          return moved_temporarily;
        case HTTPSTATE(NOT_MODIFIED):
          return not_modified;
        case HTTPSTATE(BAD_REQUEST):
          return bad_request;
        case HTTPSTATE(UNAUTHORIZED):
          return unauthorized;
        case HTTPSTATE(FORBIDDEN):
          return forbidden;
        case HTTPSTATE(NOT_FOUND):
          return not_found;
        case HTTPSTATE(INTERNAL_SERVER_ERROR):
          return internal_server_error;
        case HTTPSTATE(NOT_IMPLEMENTED):
          return not_implemented;
        case HTTPSTATE(BAD_GATEWAY):
          return bad_gateway;
        case HTTPSTATE(SERVICE_UNAVAILABLE):
          return service_unavailable;
        default:
          return internal_server_error;
        }
      }
#undef HTTPSTATE

    } // namespace status_strings


    static http_parser_settings req_parser_settings;

    bool urlDecode(const std::string& in, std::string& out){
      out.clear();
      out.reserve(in.size());
      for(std::size_t i = 0; i < in.size(); ++i){
        if(in[i] == '%'){
          if(i + 3 <= in.size()){
            int value = 0;
            std::istringstream is(in.substr(i + 1, 2));
            if(is >> std::hex >> value){
              out += static_cast<char>(value);
              i += 2;
            } else{
              return false;
            }
          } else{
            return false;
          }
        } else if(in[i] == '+'){
          out += ' ';
        } else{
          out += in[i];
        }
      }
      return true;
    }

    std::string urlEncode(const std::string& str){
      std::stringstream ss;
      size_t length = str.length();
      for(size_t i = 0; i < length; i++){
        if(isalnum((unsigned char)str[i]) || (str[i] == '-') ||  (str[i] == '_') ||(str[i] == '.') || (str[i] == '~')){
          ss << str[i];
        //} else if(str[i] == ' '){
        //  ss << '+';
        } else{
          ss << '%';
          ss << std::hex << ((unsigned char)str[i] >> 4);
          ss << std::hex << ((unsigned char)str[i] % 16);
        }
      }
      return ss.str();
    }

    void HttpBlock::initHttpParserSetting(){
      req_parser_settings.on_message_begin = [](http_parser* /*parser*/){ 
        //LOGDEBUG("***MESSAGE BEGIN***");
        return 0;
      };
      req_parser_settings.on_url = [](http_parser* parser, const char* at, size_t length){
        std::string urlData(at, length);
        HttpBlock *client = (HttpBlock*)parser->data;
        if (client){ 
          std::string url;
          urlDecode(urlData, url);
          if(url.empty() ||url[0] != '/' || url.find("..") != std::string::npos){
            client->setError(true);
            return 0;
          }
          url.erase(url.begin());

          auto pos = url.find_first_of('?');
          if (pos != std::string::npos){
            client->m_url = url.substr(0, pos);
            client->m_request = url.substr(pos + 1);
          } else{
             client->m_url = url;
          }
          
          //LOGDEBUG("se:", client->session()," url:", client->m_url, "  request:", client->m_request);
        }
        return 0;
      };
      req_parser_settings.on_header_field = [](http_parser* /*parser*/, const char* /*at*/, size_t /*length*/){
        //LOGDEBUG("Header field: ", std::string(at, length));
        return 0;
      };
      req_parser_settings.on_header_value = [](http_parser* /*parser*/, const char* /*at*/, size_t /*length*/){
        //LOGDEBUG("Header value:", std::string(at, length));
        return 0;
      };
      req_parser_settings.on_headers_complete = [](http_parser* /*parser*/){
        //LOGDEBUG("***HEADERS COMPLETE***");
        return 0;
      };
      req_parser_settings.on_body = [](http_parser* parser, const char* at, size_t length){
        //LOGDEBUG("Body:", std::string(at, length));
        HttpBlock *client = (HttpBlock*)parser->data;
        if(at && client){
          client->m_request.append(at, length);
        }
        return 0;
      };
      req_parser_settings.on_message_complete =[](http_parser* parser) {
        //LOGDEBUG("***MESSAGE COMPLETE***");
        HttpBlock *client = (HttpBlock*)parser->data;
        if(client){client->m_done = true;}
        //size_t total_len = client->m_request.length();
        //LOGDEBUG("total length parsed:", total_len);
        return 0;
      };
    }

#define CRLF "\r\n"              

    HttpBlock::HttpBlock(SessionId s, bool server): BlockBase(s), m_parser(new http_parser){
      m_parser->data = this; 
      //HTTP_RESPONSE client use 
      //HTTP_REQUEST  server use
      http_parser_init(m_parser.get(), server ? HTTP_REQUEST : HTTP_RESPONSE);
    }

    HttpBlock::HttpBlock(SessionId s, uint32 code, const std::string& response): BlockBase(s){
      if (code != HTTP_STATUS_OK){
        auto stateContent = std::move(stock_replies::to_string(code));
        //status
        m_request += status_strings::to_string(code);
        //headers
        m_request += "Content-Length: ";
        m_request += std::to_string(stateContent.length());
        m_request += CRLF;
        m_request += "Content-Type: text/html";
        m_request += CRLF;
        //content
        m_request += CRLF;
        m_request += stateContent;
      } else{
        //status
        m_request += status_strings::to_string(code);
        //headers
        m_request += "Content-Length: ";
        m_request += std::to_string(response.length());
        m_request += CRLF;
        m_request += "Content-Type: application/json; charset=utf-8";
        m_request += CRLF;
        //content
        m_request += CRLF;
        m_request += response;
      }
    }
     
    const char* HttpMethod[HttpBlock::AC_MAX] = {"DELETE", "GET", "HEAD", "POST","PUT",};
    
    HttpBlock::HttpBlock(SessionId s, uint32 method, const std::string& host,const std::string& path, const std::string& cmd) : BlockBase(s), m_url(path){
      if(!checkAction(method)){MYASSERT(false); method = AC_GET ;}
     
      switch(method){
      case AC_GET:{
          m_url =  urlEncode(path + "?" +  cmd);
          std::stringstream request_stream;
          request_stream << HttpMethod[method] << " /" << m_url << " HTTP/1.1" << CRLF;
          request_stream << "Host: " << host << CRLF;
          request_stream << "Accept: */*"<< CRLF;
          request_stream << "Connection: close" << CRLF << CRLF;
          m_request = request_stream.str();
        } break;
      case AC_POST:{
          std::stringstream request_stream;
          m_url =  urlEncode(path + "?");
          request_stream << HttpMethod[method] << " /" <<  m_url << " HTTP/1.1" << CRLF;
          request_stream << "Accept: */*"<< CRLF ;
          request_stream << "Accept-Charset:gb2312,utf-8;" << CRLF;
          request_stream << "Host: " << host << CRLF ;
          request_stream << "Connection: close" << CRLF ;
          request_stream << "Content-Type: application/json; charset=utf-8" << CRLF;
          request_stream << "Content-Length: " << std::to_string(cmd.length())  << CRLF << CRLF;
          request_stream << cmd ;
          m_request = request_stream.str();
        }break;
      default:
        break;
      }
      
    }

    HttpBlock::~HttpBlock(){
      if (m_parser){ m_parser.reset();}
    }

    bool HttpBlock::done() const{
      return m_done;
    }

    bool HttpBlock::recv(BufferPointer& buf){
      size_t nread = buf->needReadLength();
      int64 parsed = (int64)http_parser_execute(m_parser.get(), &req_parser_settings, buf->readData(),nread);
      if(m_parser->upgrade){
        LOGINFO("We do not support upgrades yet");
        setError(true);
      } else if(static_cast<size_t>(parsed) != nread){
        LOGINFO("parsed incomplete data:", parsed, "/", nread, " bytes parsed");
        auto error = http_errno_description((http_errno)m_parser->http_errno);
        LOGINFO("\n***", error, " ***\n");
        setError(true);
      }

      if (parsed > 0){
        buf->readData(static_cast<size_t>(parsed));
        buf->clearReadBuffer();
      }
      return true;
    }

    BlockBase* HttpBlock::clone( SessionId /*s*/){ 
      MYASSERT(false, "http block can not clone");
      return nullptr;
    }

    bool HttpBlock::readBuffer(NetBuffer& buffer,bool /*force*/){
     if(buffer.tailData(m_request.data(), m_request.length())){
       m_done = true;
       return true;
     }
     return false;
    }

    size_t HttpBlock::length(){
      return m_parser == nullptr ? m_request.length() : m_parser->content_length;
    }

    bool HttpBlock::readComplete() const{
      return m_done;
    }

    const std::string& HttpBlock::content() const{
      return m_request;
    }

    const std::string& HttpBlock::url() const{
      return m_url;
    }






  }
}