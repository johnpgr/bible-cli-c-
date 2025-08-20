#pragma once

#include "allocator.h"
#include "array.h"
#include "defs.h"
#include <cstring>
#include <optional>
#include <print>

enum CLIOptionType { BOOL, STRING, I8, I16, I32, I64, U8, U16, U32, U64, F32, F64 };

struct CLIOptionValue {
    CLIOptionType type;
    union {
        bool bool_val;
        const char* string_val;
        i8 i8_val;
        i16 i16_val;
        i32 i32_val;
        i64 i64_val;
        u8 u8_val;
        u16 u16_val;
        u32 u32_val;
        u64 u64_val;
        f32 f32_val;
        f64 f64_val;
    };

    static CLIOptionValue create_bool(bool value) {
        return CLIOptionValue{.type = CLIOptionType::BOOL, .bool_val = value};
    }

    static CLIOptionValue create_string(const char* value) {
        return CLIOptionValue{.type = CLIOptionType::STRING, .string_val = value};
    }

    template <IntegerType T> static CLIOptionValue create_integer(T value) {
        if constexpr (std::same_as<T, i8>) {
            return CLIOptionValue{.type = CLIOptionType::I8, .i8_val = value};
        } else if constexpr (std::same_as<T, i16>) {
            return CLIOptionValue{.type = CLIOptionType::I16, .i16_val = value};
        } else if constexpr (std::same_as<T, i32>) {
            return CLIOptionValue{.type = CLIOptionType::I32, .i32_val = value};
        } else if constexpr (std::same_as<T, i64>) {
            return CLIOptionValue{.type = CLIOptionType::I64, .i64_val = value};
        } else if constexpr (std::same_as<T, u8>) {
            return CLIOptionValue{.type = CLIOptionType::U8, .u8_val = value};
        } else if constexpr (std::same_as<T, u16>) {
            return CLIOptionValue{.type = CLIOptionType::U16, .u16_val = value};
        } else if constexpr (std::same_as<T, u32>) {
            return CLIOptionValue{.type = CLIOptionType::U32, .u32_val = value};
        } else if constexpr (std::same_as<T, u64>) {
            return CLIOptionValue{.type = CLIOptionType::U64, .u64_val = value};
        }
    }

    static CLIOptionValue create_float(f32 val) {
        return CLIOptionValue{.type = CLIOptionType::F32, .f32_val = val};
    }

    static CLIOptionValue create_double(f64 val) {
        return CLIOptionValue{.type = CLIOptionType::F32, .f64_val = val};
    }
};

struct CLIOption {
    const char* short_name;
    const char* long_name;
    const char* description;
    CLIOptionType type;
    bool allow_multiple;
    ArrayList<CLIOptionValue> values;

    static CLIOption init(
        Allocator& allocator,
        const char* short_name,
        const char* long_name,
        const char* description,
        CLIOptionType type,
        bool allow_multiple = false,
        usize max_values = 10
    ) {
        auto values = ArrayList<CLIOptionValue>::init(allocator, max_values);

        return CLIOption{
            .short_name = short_name,
            .long_name = long_name,
            .description = description,
            .type = type,
            .allow_multiple = allow_multiple,
            .values = values,
        };
    }

    void deinit() { values.deinit(); }

    static const char* parse_name(const char* arg) {
        if (arg[0] == '-' && arg[1] == '-') {
            return arg + 2; // Skip "--"
        } else if (arg[0] == '-') {
            return arg + 1; // Skip "-"
        }

        return arg;
    }

    bool equals(const char* name_short_or_long) {
        bool eq_short = strcmp(parse_name(short_name), name_short_or_long) == 0;
        bool eq_long = strcmp(parse_name(long_name), name_short_or_long) == 0;

        return (short_name && eq_short) || (long_name && eq_long);
    }

    bool has_value() { return type != CLIOptionType::BOOL; }

    bool add_value(const char* raw_value) {
        if (!allow_multiple && values.len > 0) return false; // Only one value allowed

        auto parsed_value = parse_value(raw_value);
        if (!parsed_value.has_value()) {
            std::println("Failed to parse value: {} for command: {}", raw_value, long_name);
            return false;
        }

        return values.append(parsed_value.value());
    }

    void set_bool_value(bool value) {
        if (type == CLIOptionType::BOOL) {
            values.clear();
            values.append(CLIOptionValue::create_bool(value));
        }
    }

    // Get single value (first one if multiple)
    std::optional<CLIOptionValue> get_first_value() {
        if (values.len == 0) return std::nullopt;
        return values.items[0];
    }

    ArrayList<CLIOptionValue>& get_all_values() { return values; }

    std::optional<bool> get_first_bool() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::BOOL) {
            return val->bool_val;
        }

        return std::nullopt;
    }

    std::optional<const char*> get_first_string() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::STRING) {
            return val->string_val;
        }

        return std::nullopt;
    }

    std::optional<i8> get_first_i8() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::I8) {
            return val->i8_val;
        }

        return std::nullopt;
    }

    std::optional<u8> get_first_u8() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::U8) {
            return val->u8_val;
        }

        return std::nullopt;
    }

    std::optional<i16> get_first_i16() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::I16) {
            return val->i16_val;
        }

        return std::nullopt;
    }

    std::optional<u16> get_first_u16() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::U16) {
            return val->u16_val;
        }

        return std::nullopt;
    }

    std::optional<i32> get_first_i32() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::I32) {
            return val->i32_val;
        }

        return std::nullopt;
    }

    std::optional<u32> get_first_u32() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::U32) {
            return val->u32_val;
        }

        return std::nullopt;
    }

    std::optional<i64> get_first_i64() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::I64) {
            return val->i64_val;
        }

        return std::nullopt;
    }

    std::optional<u64> get_first_u64() {
        auto val = get_first_value();
        if(val.has_value() && val->type == CLIOptionType::U64) {
            return val->u64_val;
        }

        return std::nullopt;
    }

    void get_all_strings(ArrayList<const char*>& out) {
        for (auto& val : values) {
            if (val.type == CLIOptionType::STRING) {
                out.append(val.string_val);
            }
        }
    }

  private:
    std::optional<CLIOptionValue> parse_value(const char* raw_value) {
        switch (type) {
            case BOOL: {
                return CLIOptionValue::create_bool(true);
            } break;
            case STRING: {
                return CLIOptionValue::create_string(raw_value);
            } break;
            case I8: {
                auto num = int_from_str<i8>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case I16: {
                auto num = int_from_str<i16>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case I32: {
                auto num = int_from_str<i32>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case I64: {
                auto num = int_from_str<i64>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case U8: {
                auto num = int_from_str<u8>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case U16: {
                auto num = int_from_str<u16>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case U32: {
                auto num = int_from_str<u32>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;
            case U64: {
                auto num = int_from_str<u64>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_integer(num.value());
            } break;

            case F32: {
                auto num = float_from_str<f32>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_float(num.value());
            } break;
            case F64: {
                auto num = float_from_str<f64>(raw_value);
                if (!num.has_value()) {
                    return std::nullopt;
                }
                return CLIOptionValue::create_double(num.value());
            } break;
        }
    }
};

struct CLICommand;

typedef bool (*CommandCallback)(CLICommand& command, void* user_data);

struct CLICommand {
    const char* name;
    const char* description;
    ArrayList<CLIOption> options;
    CommandCallback callback;
    void* user_data;

    static CLICommand init(
        Allocator& allocator,
        char* name,
        char* description,
        CommandCallback callback = nullptr,
        void* user_data = nullptr,
        usize max_options = 10
    ) {
        auto options = ArrayList<CLIOption>::init(allocator, max_options);

        return CLICommand{
            .name = name,
            .description = description,
            .options = options,
            .callback = callback,
            .user_data = user_data,
        };
    }

    void deinit() { 
        for (auto& opt : options) {
            opt.deinit();
        }
        options.deinit(); 
    }

    bool add_option(CLIOption& option) { return options.append(option); }

    std::optional<CLIOption&> get_option(const char* name) {
        for (auto& opt : options) {
            if (opt.equals(name)) return opt;
        }
        return std::nullopt;
    }

    void set_option_value(const char* name, const char* value) {
        for (auto& opt : options) {
            if (opt.equals(name)) {
                opt.value = value;
                return;
            }
        }
    }

    bool execute() {
        if (callback) {
            return callback(*this, user_data);
        }
        return true;
    }
};

struct CLIParser {
    const char* program_name;
    std::optional<CLICommand> current_command;
    std::optional<CLICommand> main_command;
    ArrayList<CLICommand> commands;

    static CLIParser init(Allocator& allocator, char* program_name, i32 max_commands = 20) {
        auto commands = ArrayList<CLICommand>::init(allocator, max_commands);

        return CLIParser{
            .program_name = program_name,
            .current_command = std::nullopt,
            .main_command = std::nullopt,
            .commands = commands,
        };
    }

    void deinit() {
        for (auto& cmd : commands) {
            cmd.deinit();
        }

        if (main_command.has_value()) {
            main_command->deinit();
        }

        commands.deinit();
    }

    bool add_command(CLICommand command) { return commands.append(command); }

    void set_main_command(CLICommand command) { main_command = command; }

    int parse_and_execute(i32 argc, char* argv[]) {
        if (!parse(argc, argv)) {
            return 1;
        }

        if (current_command && current_command->callback) {
            return current_command->execute() ? 0 : 1;
        }

        return 0;
    }

    bool parse(i32 argc, char* argv[]) {
        // If no arguments or first argument is an option, use main command
        if (argc < 2 || is_option(argv[1])) {
            if (!main_command.has_value()) {
                print_help();
                return false;
            }

            current_command = main_command;

            // Parse options starting from argv[1] for main command
            for (i32 i = 1; i < argc; i++) {
                if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                    print_command_help(current_command.value());
                    return false;
                }

                if (is_option(argv[i])) {
                    if (!parse_option(argv[i], current_command.value())) {
                        return false;
                    }

                    // Check if this option expects a value
                    const char* option_name = CLIOption::parse_name(argv[i]);
                    for (auto& option : current_command->options) {
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

        // Look for named command
        auto found_command = find_command(argv[1]);
        if (!found_command.has_value()) {
            std::println("Unknown command: {}", argv[1]);
            print_help();
            return false;
        }

        current_command = found_command;

        // Check for help option
        for (i32 i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                print_command_help(current_command.value());
                return false;
            }
        }

        // Parse options starting from argv[2] for named commands
        for (i32 i = 2; i < argc; i++) {
            if (is_option(argv[i])) {
                if (!parse_option(argv[i], current_command.value())) {
                    return false;
                }

                const char* option_name = CLIOption::parse_name(argv[i]);
                for (auto& option : current_command->options) {
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
        std::println("Usage {} [options]", program_name);

        if (main_command.has_value()) {
            std::println("       {} <command> [options]\n", program_name);
        } else {
            std::println("       {} <command> [options]\n", program_name);
        }

        if (main_command.has_value()) {
            std::println("Main command:");
            std::println("{:<15} {}\n", "(default)", main_command->description);
        }

        if (commands.len > 0) {
            std::println("Commands:");
            for (auto& cmd : commands) {
                std::println("{:<15} {}", cmd.name, cmd.description);
            }
        }

        std::println(
            "\nUse '{} --help' or '{} <command> --help' for more information.\n",
            program_name,
            program_name
        );
    }

    void print_command_help(CLICommand& command) {
        std::println("Usage: {} {} [options]\n", program_name, command.name);
        std::println("{}\n", command.description);

        if (command.options.len > 0) {
            std::println("Options:");

            for (auto& option : command.options) {
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

                std::println("\n{}", option.description);
            }
        }
    }

  private:
    bool parse_option(char* arg, CLICommand& command) {
        const char* option_name = CLIOption::parse_name(arg);

        // Find the option in the command
        for (auto& option : command.options) {
            if (option.equals(option_name)) {
                if (!option.has_value) {
                    command.set_option_value(option_name, "true");
                }
                return true;
            }
        }

        std::println("Unknown option: {}", arg);
        return false;
    }

    std::optional<CLICommand> find_command(char* name) {
        for (auto& command : commands) {
            if (strcmp(command.name, name) == 0) {
                return command;
            }
        }

        return std::nullopt;
    }

    bool is_option(char* arg) { return arg[0] == '-'; }
};
