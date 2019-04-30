#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

vector<string> split(const string &str, char delimiter) {
    vector<string> splited;
    if (str.empty()) {
        return splited;
    }
    size_t i = 0, j = str.find(delimiter);
    while (true) {
        if (j == string::npos) {
            splited.push_back(str.substr(i));
            break;
        } else if (j > i) {
            splited.push_back(str.substr(i, j - i));
        }
        i = j + 1;
        if (i == str.length()) {
            break;
        }
        j = str.find(delimiter, i);
    }
    return splited;
}

struct Command {
    vector<string> arguments;
    int redirect_type;
    string redirect_file;
};

bool parse_commands(const vector<string> &commands_str, vector<Command> &commands) {
    for (size_t i = 0; i < commands_str.size(); ++i) {
        const string &cmd_str = commands_str[i];
        size_t pos;
        if ((pos = cmd_str.find('<')) != string::npos) {
            if (i != 0) {
                return false;
            }
            if (pos == cmd_str.length() - 1) {
                return false;
            }
            Command cmd;
            cmd.arguments = split(cmd_str.substr(0, pos), ' ');
            cmd.redirect_type = 1;
            cmd.redirect_file = cmd_str.substr(pos + 1);
            if (cmd.arguments.size()) {
                commands.push_back(cmd);
            }
        } else if ((pos = cmd_str.find(">>")) != string::npos) {
            if (i != commands_str.size() - 1) {
                return false;
            }
            if (pos >= cmd_str.length() - 2) {
                return false;
            }
            Command cmd;
            cmd.arguments = split(cmd_str.substr(0, pos), ' ');
            cmd.redirect_type = 2;
            cmd.redirect_file = cmd_str.substr(pos + 2);
            if (cmd.arguments.size()) {
                commands.push_back(cmd);
            }
        } else if ((pos = cmd_str.find('>')) != string::npos) {
            if (i != commands_str.size() - 1) {
                return false;
            }
            if (pos == cmd_str.length() - 1) {
                return false;
            }
            Command cmd;
            cmd.arguments = split(cmd_str.substr(0, pos), ' ');
            cmd.redirect_type = 3;
            cmd.redirect_file = cmd_str.substr(pos + 1);
            if (cmd.arguments.size()) {
                commands.push_back(cmd);
            }
        } else {
            Command cmd;
            cmd.arguments = split(cmd_str, ' ');
            cmd.redirect_type = 0;
            if (cmd.arguments.size()) {
                commands.push_back(cmd);
            }
        }
    }
    return true;
}

int main() {
    while (true) {
        cout << "# ";
        string line;
        getline(cin, line);
        const vector<string> &commands_str = split(line, '|');
        vector<Command> commands;
        if (!parse_commands(commands_str, commands)) {
            cerr << "syntax error\n";
            continue;
        }
        if (commands.empty()) {
            continue;
        }
        if (commands.size() == 1) {
            const Command &cmd = commands[0];
            if (cmd.arguments[0] == "cd") {
                if (chdir(cmd.arguments[1].c_str()) == -1) {
                    cerr << "cannot chdir to \"" << cmd.arguments[1] << "\"\n";
                }
                continue;
            } else if (cmd.arguments[0] == "export") {
                if (cmd.arguments.size() <= 1) {
                    cerr << "syntax error\n";
                    continue;
                }
                const vector<string> &kv = split(cmd.arguments[1], '=');
                if (kv.size() != 2) {
                    cerr << "syntax error\n";
                    continue;
                }
                if (setenv(kv[0].c_str(), kv[1].c_str(), 1) == -1) {
                    cerr << "cannot setenv\n";
                }
                continue;
            } else if (cmd.arguments[0] == "exit") {
                return 0;
            }
        }
        pid_t pid;
        size_t i = 0;
        int in = STDIN_FILENO;
        if (commands[0].redirect_type == 1) {
            in = open(commands[0].redirect_file.c_str(), O_RDONLY);
            if (in == -1) {
                cerr << "failed to open file\n";
                continue;
            }
        }
        int fd[2];
        for (; i < commands.size() - 1; ++i) {
            pipe(fd);
            pid = fork();
            if (pid == 0) {
                if (in != STDIN_FILENO) {
                    dup2(in, STDIN_FILENO);
                }
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                break;
            }
            close(fd[1]);
            in = fd[0];
        }
        if (pid != 0) {
            pid = fork();
            if (pid == 0) {
                if (in != STDIN_FILENO) {
                    dup2(in, STDIN_FILENO);
                }
                if (commands[commands.size()-1].redirect_type == 2) {
                    int out = open(commands[commands.size()-1].redirect_file.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0666);
                    if (out == -1) {
                        cerr << "failed to open file\n";
                        return 0;
                    }
                    dup2(out, STDOUT_FILENO);
                } else if (commands[commands.size()-1].redirect_type == 3) {
                    int out = open(commands[commands.size()-1].redirect_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
                    if (out == -1) {
                        cerr << "failed to open file\n";
                        return 0;
                    }
                    dup2(out, STDOUT_FILENO);
                }
            } else {
                ++i;
            }
        }
        if (i < commands.size()) {
            if (commands[i].arguments[0] == "pwd") {
                char dirname[4096];
                getcwd(dirname, 4096);
                cout << dirname << "\n";
                return 0;
            }
            char **argv = new char *[commands[i].arguments.size() + 1];
            size_t j = 0;
            for (; j < commands[i].arguments.size(); ++j) {
                argv[j] = const_cast<char *>(commands[i].arguments[j].c_str());
            }
            argv[j] = NULL;
            if (execvp(commands[i].arguments[0].c_str(), argv) == -1) {
                cerr << "failed to execute \"" << commands_str[i] << "\"\n";
                return 0;
            }
        }
        for (size_t j = 0; j < commands.size(); ++j) {
            wait(NULL);
        }
    }
}