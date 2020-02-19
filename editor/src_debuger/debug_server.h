#pragma once
#include <deque>

#include <http_server/http_server.h>
#include <http_server/http_connection.h>
#include <behavior/btree_trace.h>
#include <any_container/decode.h>

namespace spiritsaway::behavior_tree::editor
{
	using namespace spiritsaway::http;
	class debug_connection : public spiritsaway::http::http_connection
	{
		
	private:
		std::deque<behavior_tree::common::agent_cmd_detail>& _cmd_queue;
	public:
		static std::shared_ptr<debug_connection> create(asio::ip::tcp::socket&& _in_client_socket, std::shared_ptr<spdlog::logger> logger, std::uint32_t in_connection_idx, std::deque<behavior_tree::common::agent_cmd_detail>& _in_cmd_queue)

		{
			return std::make_shared<debug_connection>(std::move(_in_client_socket), logger, in_connection_idx);
		}
		debug_connection(asio::ip::tcp::socket&& in_client_socket, std::shared_ptr<spdlog::logger> logger, std::uint32_t in_connection_idx, std::deque<behavior_tree::common::agent_cmd_detail>& _in_cmd_queue)
			: http_connection(std::move(in_client_socket), logger, in_connection_idx, 1, "debug_connection")
			, _cmd_queue(_in_cmd_queue)
		{

		}

		void on_client_data_body_read(const http_request_header& _header, std::string_view _content)
		{
			std::string error_desc = "";
			std::string entity_id = "";
			using temp_cmd_type = std::tuple<std::uint64_t, std::uint32_t, spiritsaway::serialize::any_vector>;
			std::vector<temp_cmd_type> cmds;
			while (true)
			{
				if (_header.method() != "POST" || _header.path_and_query() != "/post/ai_debug/")
				{
					error_desc = "the method should be POST and  post path should be /post/ai_debug";
					break;
				}
				if (!json::accept(_content))
				{
					error_desc = "the content should be json str";
					break;
				}
				json post_data = json::parse(_content);
				auto entity_id_iter = post_data.find("entity_id");
				if (entity_id_iter == post_data.end())
				{
					error_desc = "entity_id should be in the data";
					break;
				}
				if (!entity_id_iter->is_string())
				{
					error_desc = "entity_id should be str";
					break;
				}
				entity_id = entity_id_iter->get<std::string>();
				auto cmd_iter = post_data.find("cmds");
				if (cmd_iter == post_data.end())
				{
					error_desc = "cmds should be in data";
					break;
				}
				if (!cmd_iter->is_array())
				{
					error_desc = "cmds should be array";
					break;
				}
				if (!spiritsaway::serialize::decode(*cmd_iter, cmds))
				{
					error_desc = "cmds format not match";
					break;
				}

				for (auto one_cmd : cmds)
				{
					auto[ts, cmd_int, params] = one_cmd;
					behavior_tree::common::agent_cmd_detail cur_detail;
					cur_detail = make_tuple(ts, static_cast<behavior_tree::common::agent_cmd>(cmd_int), params);
					_cmd_queue.push_back(cur_detail);
				}
				break;
			}

			http_response_header _cur_response;
			_cur_response.set_version("1.0");
			if (!error_desc.empty())
			{
				_cur_response.status_code(reply_status::bad_request);
				_cur_response.status_description(error_desc);
			}
			else
			{
				_cur_response.status_code(reply_status::ok);
				_cur_response.status_description("OK");
			}
			_cur_response.add_header_value("Content-Type", "text/html");
			_cur_response.add_header_value("Server", "Http Server");
			auto request_str = _header.encode_to_data();
		}
	};
}