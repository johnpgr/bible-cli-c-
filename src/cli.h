#pragma once

#include "allocator.h"
#include "defs.h"
#include <optional>
#include <cstring>
#include <print>

struct CLIOption {
    const char* short_name;
    const char* long_name;
    const char* description;
    bool has_value;
    const char* value;

    static CLIOption init(
        char* short_name,
        char* long_name,
        char* description,
        bool has_value
    ) {
        return CLIOption{
            .short_name = short_name,
            .long_name = long_name,
            .description = description,
            .has_value = has_value,
            .value = nullptr
        };
    }

    bool equals(char* name_short_or_long) {
        return (short_name && strcmp(short_name, name_short_or_long) == 0) ||
               (long_name && strcmp(long_name, name_short_or_long) == 0);
    }
};

struct CLICommand {
    const char* name;
    const char* description;
    CLIOption* options;
    i32 option_count;
    i32 max_options;

    static CLICommand
    init(Allocator& allocator, char* name, char* description, i32 max_options = 10) {
        auto options = allocator.alloc_array<CLIOption>(max_options);

        return CLICommand{
            .name = name,
            .description = description,
            .options = options,
            .option_count = 0,
            .max_options = max_options,
        };
    }

    void deinit(Allocator& allocator) { 
        allocator.free_array(options, max_options); 
    }

    bool add_option(CLIOption& option) {
        if (option_count >= max_options) {
            return false;
        }

        options[option_count] = option;
        option_count++;

        return true;
    }

    std::optional<const char*> get_option_value(char* name) {
        for (i32 i = 0; i < option_count; i++) {
            if (options[i].equals(name)) {
                return options[i].value;
            }
        }

        return std::nullopt;
    }

    bool has_option(char* name) { 
        return get_option_value(name).has_value(); 
    }
};

struct CLIParser {
    const char* program_name;
    std::optional<CLICommand> current_command;

    CLICommand* commands;
    i32 command_count;
    i32 max_commands;

    static CLIParser init(Allocator& allocator, char* program_name, i32 max_commands = 20) {
        auto commands = allocator.alloc_array<CLICommand>(max_commands);

        return CLIParser{
            .program_name = program_name,
            .current_command = std::nullopt,
            .commands = commands,
            .command_count = 0,
            .max_commands = max_commands,
        };
    }

    void deinit(Allocator& allocator) {
        for (i32 i = 0; i < command_count; i++) {
            commands[i].deinit(allocator);
        }

        allocator.free_array(commands, max_commands);
    }

    bool add_command(CLICommand& command) {
        if (command_count >= max_commands) {
            return false;
        }

        commands[command_count] = command;
        command_count++;

        return true;
    }

    bool parse(i32 argc, char* argv[]) {
        if (argc < 2) {
            print_help();
            return false;
        }

        // Find le command
        auto found_command = find_command(argv[1]);
        if (!found_command.has_value()) {
            std::println("Unknown command: {}", argv[1]);
            print_help();
            return false;
        }

        current_command = found_command;

        // Parse le options
        for (i32 i = 2; i < argc; i++) {
            if (is_option(argv[i])) {
                if (!parse_option(argv[i], current_command.value())) {
                    return false;
                }

                // Check if this option expects a value
                char* option_name = get_option_name(argv[i]);
                for (i32 j = 0; j < current_command->option_count; j++) {
                    CLIOption option = current_command->options[j];

                    if (option.equals(option_name)) {
                        if (option.has_value && i + 1 < argc && !is_option(argv[i + 1])) {
                            option.value = argv[++i];
                        }
                        break;
                    }
                }
            }
        }

        return true;
    }

    void print_help() {
        std::println("Usage {} <command> [options]\n", program_name);
        std::println("Commands:");

        for (i32 i = 0; i < command_count; i++) {
            auto cmd = commands[i];

            std::println("  {} {}", cmd.name, cmd.description);
        }

        std::println("\nUse '{} <command> --help' for more information on a command.\n", program_name);
    }

    void print_command_help(CLICommand& command) {
        std::println("Usage: {} {} [options]\n", program_name, command.name);
        std::println("{}\n", command.description);

        if (command.option_count > 0) {
            std::println("Options:");

            for (i32 i = 0; i < command.option_count; i++) {
                auto option = command.options[i];
                std::print("  ");

                if (option.short_name) {
                    std::print("-{}", option.short_name);
                    if (option.long_name) {
                        std::print(", ");
                    }
                }

                if (option.long_name) {
                    std::print("--{}", option.long_name);
                }

                if (option.has_value) {
                    std::print(" <value>");
                }

                std::println("\n    {}", option.description);
            }
        }
    }

private:
    bool parse_option(char* arg, CLICommand& command) {
        char* option_name = get_option_name(arg);

        // Find the option in the command
        for (i32 i = 0; i < command.option_count; i++) {
            auto option = command.options[i];
            if (option.equals(option_name)) {
                if (!option.has_value) {
                    option.value = "true";
                }
                return true;
            }
        }

        std::println("Unknown option: {}", arg);

        return false;
    }

    std::optional<CLICommand> find_command(char* name) {
        for (i32 i = 0; i < command_count; i++) {
            if (strcmp(commands[i].name, name) == 0) {
                return commands[i];
            }
        }

        return std::nullopt;
    }

    bool is_option(char* arg) { 
        return arg[0] == '-'; 
    }

    char* get_option_name(char* arg) {
        if (arg[0] == '-' && arg[1] == '-') {
            return arg + 2; // Skip "--"
        } else if (arg[0] == '-') {
            return arg + 1; // Skip "-"
        }

        return arg;
    }
};
