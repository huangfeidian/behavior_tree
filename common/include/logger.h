#ifndef LOGGER_H
#define LOGGER_H
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

namespace behavior_tree::common
{
    class logger_mgr
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> _cache;
    public:
        std::shared_ptr<spdlog::logger> create_logger(const std::string& name)
        {
            auto logger_iter = _cache.find(name);
            if(logger_iter != _cache.end())
            {
                return logger_iter->second;
            }
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::debug);
            std::string pattern = "[" + name + "] [%^%l%$] %v";
            console_sink->set_pattern(pattern);

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log", true);
            file_sink->set_level(spdlog::level::trace);
            auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ console_sink, file_sink });
            logger->set_level(spdlog::level::trace);
            _cache[name] = logger;
            return logger;
        }
    private:
        logger_mgr()
        {

        }
    public:
        static logger_mgr& instance()
        {
            static logger_mgr _instance;
            return _instance;
        }
    };
}
#endif // LOGGER_H
