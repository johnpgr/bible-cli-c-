#include "cli.h"
#include "number.h"
#include "string.h"
#include <format>

struct Application {
    // The main allocator of the app
    Allocator& allocator;
    // The file path to bible xml file
    std::optional<string> file_path;
    // The book name. Examples: "John", "1 Corinthians", "1 Corinthians" The name should be in the
    // same language as the Bible file.
    std::optional<string> book;
    // The chapter number
    std::optional<usize> chapter;
    /// The verses range that is gonna be read
    /// If not set, the whole chapter is read
    ArrayList<usize> verses;

    static Application init(Allocator& allocator) {
        return Application{
            .allocator = allocator,
            .file_path = std::nullopt,
            .book = std::nullopt,
            .chapter = std::nullopt,
            .verses = ArrayList<usize>::init(allocator, 2)
        };
    }

    void deinit() { verses.deinit(); }

    bool parse_verses(string verse_str) {
        if (string_contains(verse_str, "-")) { // Range case
            char start_buffer[32];
            char end_buffer[32];

            i32 dash_pos = string_find(verse_str, "-");
            if (!string_substring(verse_str, 0, dash_pos, start_buffer, sizeof(start_buffer)) ||
                !string_substring(
                    verse_str,
                    dash_pos + 1,
                    (i32)strlen(verse_str) - dash_pos - 1,
                    end_buffer,
                    sizeof(end_buffer)
                )) {
                return false;
            }

            auto start = int_from_str<usize>(start_buffer);
            auto end = int_from_str<usize>(end_buffer);
            if (!start.has_value() || !end.has_value() || start.value() == 0 || end.value() == 0 ||
                start.value() > end.value()) {
                return false;
            }

            verses.append(start.value());
            verses.append(end.value());
        } else { // Single verse case
            auto verse = int_from_str<usize>(verse_str);

            if (!verse.has_value()) {
                return false;
            }

            if (verse.value() == 0) return false;
            verses.append(verse.value());
        }

        return true;
    }
};

bool main_command_handler(CLICommand& command, void* user_data) {
    auto app = (Application*)user_data;

    // Handle book option
    auto book_opt = command.get_option("book");
    if (!book_opt.has_value() || !book_opt->value.has_value()) {
        std::println("Error: Book name is required. Use -b or --book to specify.");
        return false;
    }
    app->book = book_opt->value.value();

    // Handle chapter option
    auto chapter_opt = command.get_option("chapter");
    if (!chapter_opt.has_value() || !chapter_opt->value.has_value()) {
        std::println("Error: Chapter number is required. Use -c or --chapter to specify.");
        return false;
    }

    auto chapter_parsed = int_from_str<usize>(chapter_opt->value.value());
    if (!chapter_parsed.has_value()) {
        std::println("Error: Invalid chapter number '{}'", chapter_opt->value.value());
        return false;
    }
    app->chapter = chapter_parsed.value();

    // Handle verse option (optional)
    auto verse_opt = command.get_option("verse");
    if (verse_opt.has_value() && verse_opt->value.has_value()) {
        if (!app->parse_verses(verse_opt->value.value()) || app->verses.len == 0) {
            std::println("Error: Invalid verse number or range '{}'", verse_opt->value.value());
            return false;
        }
    }

    std::println("Reading {} chapter {}", app->book.value(), app->chapter.value());

    if (app->verses.len > 0) {
        auto verse_start = app->verses[0];
        auto verse_end = app->verses[1];

        if (verse_start.has_value() && verse_end.has_value()) {
            std::println("Verses: {}..{}", verse_start.value(), verse_end.value());
        } else if (verse_start.has_value()) {
            std::println("Verse: {}", verse_start.value());
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    ArenaAllocator arena = ArenaAllocator::init(PageAllocator::init(), 4096, MB(8));
    Allocator allocator = arena.allocator();
    defer { arena.deinit(); };

    CLIParser parser = CLIParser::init(allocator, "Bible Reader");
    defer { parser.deinit(); };

    Application app = Application::init(allocator);
    defer { app.deinit(); };

    CLICommand main_command = CLICommand::init(
        allocator,
        nullptr,
        "Read Bible verses by book, chapter and verse",
        &main_command_handler,
        &app
    );

    // TODO: the file should be the first parameter not option. e.g ./bible "path_to_bible_xml"
    // --book...

    CLIOption book_option = CLIOption::init("-b", "--book", "Book name (e.g. John)");
    CLIOption chapter_option = CLIOption::init("-c", "--chapter", "Chapter number");
    CLIOption verse_option = CLIOption::init("-v", "--verse", "Verse number or range");

    main_command.add_option(book_option);
    main_command.add_option(chapter_option);
    main_command.add_option(verse_option);

    parser.set_main_command(main_command);
    return parser.parse_and_execute(argc, argv);
}
