#include <unistd.h>
#include <list>

char *usage = \
	"Usage: %s filename.hpp -c MainClass [-i AlsoCopy.hpp]\n\n"\
	"-c, --class: Name of main class (required)\n" \
	"-i, --include: Copy this file\n" \
	"-help, --help: Print this message\n"

int bailError(const string &name, const string &err) {
	fprintf(stderr, "Error: %s", err.c_str());
	fprintf(stderr, usage, name.c_str());
	return 1;
}

int main(int argc, char **argv) {
	list<string> copy;
	string mainFile;
	string mainClass;

	for (int c = 1; c < argc; c++) {
		string arg = argv[c];
		if (arg == "--help" || arg == "-help") {
			printf(usage, argv[0]);
			return 0; // BAIL OUT
		} else if (arg == "-i" || arg == "--include") {
			if (c+1 >= argc)
				return bailError(argv[0], arg + " missing parameter");
			copy.push_back(argv[c+1]);
			c++;
		} else if (arg == "-c" || arg == "--class") {
			if (c+1 >= argc)
				return bailError(argv[0], arg + " missing parameter");
			if (c+1 >= argc)
				return bailError(argv[0], arg + " missing parameter");
			mainClass = argv[c+1];
			c++;
		}
	}

	return 0;
}
