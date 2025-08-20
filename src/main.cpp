#include "allocator.h"
#include "cli.h"
#include <cstdlib>

struct Application {
    // The file path to bible xml file
    const char* file_path;
    // The book name. Examples: "John", "1 Corinthians", "1 Corinthians" The name should be in the
    // same language as the Bible file.
    std::optional<const char*> book;
    // The chapter number
    std::optional<usize> chapter;
    /// The verse number or range of verses. Examples: "16", "4", "16-18"
    std::optional<const char*> verses;
};

bool main_command_handler(CLICommand& command, void* user_data) {
    auto app = (Application*)user_data;

    auto book = command.get_option_value("book");
    if (!book.has_value()) {
        std::println("Book option not found");
        return false;
    }
    app->book = book;

    auto chapter = command.get_option_value("chapter");
    if (!chapter.has_value()) {
        std::println("Chapter option not found");
        return false;
    }

    char* endptr;
    i64 num_chapter = strtol(chapter.value(), &endptr, 10);
    if (endptr == chapter.value()) {
        return false;
    } else if (*endptr != '\0') {
        std::println("Invalid character in chapter option");
        return false;
    }

    app->chapter = (usize)num_chapter;

    std::println("Application book: {}", app->book.value());
    std::println("Application chapter: {}", app->chapter.value());

    return true;
}

int main(int argc, char* argv[]) {
    ArenaAllocator arena = ArenaAllocator::init(PageAllocator::init(), 4096, MB(8));
    Allocator allocator = arena.allocator();
    defer { arena.deinit(); };

    CLIParser parser = CLIParser::init(allocator, "Bible Reader");
    defer { parser.deinit(); };

    Application app;

    CLICommand main_command = CLICommand::init(
        allocator,
        nullptr,
        "Read Bible verses by book, chapter and verse",
        &main_command_handler,
        &app
    );

    CLIOption book_option = CLIOption::init("-b", "--book", "Book name (e.g. John)", true);
    CLIOption chapter_option = CLIOption::init("-c", "--chapter", "Chapter number", true);
    CLIOption verse_option = CLIOption::init("-v", "--verse", "Verse number or range", true);

    main_command.add_option(book_option);
    main_command.add_option(chapter_option);
    main_command.add_option(verse_option);

    parser.set_main_command(main_command);
    return parser.parse_and_execute(argc, argv);
}
