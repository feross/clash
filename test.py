#!/usr/bin/python
#
# A test suite for clash. Designed as unit tests for John Ousterhout's
# implementation, but implemented at a high enough level that it can be
# applied to other implementations as well.

import os
import shutil
import subprocess
import sys
import time
import unittest

# The location of the shell to be tested. You can change this to
# /bin/bash to see if Bash passes this test suite.
shell = "./clash"
#shell = "/bin/bash"

# This variable will hold the exit status of the most recent command
# executed by "run"
status = 0

def readFile(name):
    """ Return the contents of a file. """
    f = open(name, "r")
    result = f.read()
    f.close()
    return result

def writeFile(name, contents="xyzzy"):
    """ Create a file with particular contents. """
    f = open(name, "w")
    f.write(contents)
    f.close()

def run(cmd):
    """ Invoke the test shell with the given command and return any
    output generated.
    """

    global status
    writeFile("__stdin", cmd)
    stdin = open("__stdin", "r")

    # This sometime fails under Cygwin, so try again when that happens
    try:
        stdout = open("__stdout", "w")
    except:
        time.sleep(0.01)
        stdout = open("__stdout", "w")

    status = subprocess.call(shell, stdin=stdin, stdout=stdout,
            stderr=subprocess.STDOUT)
    result = readFile("__stdout")
    stdin.close()
    stdout.close()
    os.remove("__stdin")

    # This sometime fails under Cygwin, so try again when that happens
    try:
        os.remove("__stdout")
    except:
        time.sleep(0.01)
        os.remove("__stdout")

    return result

def runWithArgs(*args):
    """ Invoke the test shell with the given set of arguments, and
    return any output generated.
    """

    global status

    # This sometime fails under Cygwin, so try again when that happens
    try:
        stdout = open("__stdout", "w")
    except:
        time.sleep(0.01)
        stdout = open("__stdout", "w")

    fullArgs = []
    fullArgs.append(shell)
    fullArgs.extend(args)
    status = subprocess.call(fullArgs, stdout=stdout,
            stderr=subprocess.STDOUT)
    result = readFile("__stdout")
    stdout.close()

    # This sometime fails under Cygwin, so try again when that happens
    try:
        os.remove("__stdout")
    except:
        time.sleep(0.01)
        os.remove("__stdout")

    return result

class TestBuiltin(unittest.TestCase):
    def tearDown(self):
        if os.path.exists("__test"):
            shutil.rmtree("__test")

    def test_cd_no_args(self):
        self.assertEqual(run("cd ~; pwd"), run("cd; pwd"))

    def test_cd_no_HOME(self):
        self.assertIn("cd: HOME not set", run("unset HOME; cd"))

    def test_cd_HOME_bad_path(self):
        self.assertIn("__bogus/foo: No such file or directory",
                run("HOME=__bogus/foo; cd"))

    def test_cd_too_many_args(self):
        self.assertIn("cd: Too many arguments", run("cd a b"))

    def test_cd_bad_path(self):
        self.assertIn("__bogus/foo: No such file or directory",
                run("cd __bogus/foo"))

    def test_cd_success(self):
        os.makedirs("__test")
        writeFile("__test/foo", "abc def");
        self.assertEqual("abc def",
                run("cd __test; /bin/cat foo"))

    def test_exit_no_args(self):
        self.assertEqual("", run("exit; echo foo"))
        self.assertEqual(0, status)

    def test_exit_with_arg(self):
        self.assertEqual("", run("exit 14; echo foo"))
        self.assertEqual(14, status)

    def test_exit_non_numeric_arg(self):
        self.assertIn("jkl: Numeric argument required",
                run("exit jkl; echo foo"))
        self.assertEqual(2, status)

    def test_exit_too_many_arguments(self):
        self.assertIn("exit: Too many arguments", run("exit 1 2 3; echo foo"))
        self.assertEqual(1, status)

class TestExpand(unittest.TestCase):
    def tearDown(self):
        if os.path.exists("__test"):
            shutil.rmtree("__test")

    def test_expandPath_no_wildcards(self):
        os.makedirs("__test/foo")
        self.assertEqual("abc def\n", run("/bin/echo abc def"))
        self.assertEqual("__test/[a]*\n", run('/bin/echo __test/"[a]*"'))

    def test_expandPath_file_matched(self):
        os.makedirs("__test/foo")
        self.assertEqual("__test/foo\n", run('/bin/echo __test/[f]*'))

    def test_expandPath_no_file_matched(self):
        os.makedirs("__test/foo");
        self.assertEqual("__test/x*\n", run('/bin/echo __test/x*'))

    def test_expandPath_multiple_files_matched(self):
        os.makedirs("__test/foo")
        writeFile("__test/foo/a.c")
        writeFile("__test/foo/b.c")
        writeFile("__test/foo/c.cc")
        result = run('/bin/echo __test/foo/*.c')
        self.assertIn("__test/foo/a.c", result)
        self.assertIn("__test/foo/b.c", result)

    def test_expandTilde(self):
        self.assertEqual("/home/ouster\n",
                run("PATH=/home/ouster; /bin/echo ~\n"))
        self.assertEqual("/home/ouster/xyz\n", run("/bin/echo ~/xyz\n"))
        self.assertEqual("/home/ouster\n", run("/bin/echo ~ouster\n"))
        self.assertEqual("/home/ouster/xyz\n", run("/bin/echo ~ouster/xyz\n"))
        self.assertEqual("~__bogus__/xyz\n", run("/bin/echo ~__bogus__/xyz\n"))

    def test_matchFiles_bad_directory(self):
        self.assertEqual("__test/bogus/*\n", run('/bin/echo __test/bogus/*'))

    def test_matchFiles_no_match_in_directory(self):
        os.makedirs("__test")
        self.assertEqual("__test/*.c\n", run('/bin/echo __test/*.c'))

    def test_matchFiles_repeated_separators(self):
        os.makedirs("__test/foo/bar")
        self.assertEqual("__test/foo\n", run('/bin/echo __t*//foo'))
        self.assertEqual("__test/foo/bar\n", run('/bin/echo __t*//f*//bar'))
        self.assertEqual("__test//foo/bar\n", run('/bin/echo __test//f*//bar'))

    def test_matchFiles_multiple_levels_of_matching(self):
        os.makedirs("__test/x1")
        os.makedirs("__test/x2")
        writeFile("__test/x1/a.c")
        writeFile("__test/x2/b.c")
        writeFile("__test/x2/c.c")
        result = run('/bin/echo __test/x?/*.c')
        self.assertIn("__test/x1/a.c", result)
        self.assertIn("__test/x2/b.c", result)
        self.assertIn("__test/x2/c.c", result)

    def test_matchString_fail_end_of_string(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__tes?/xyzq\n", run('/bin/echo __tes?/xyzq'))

    def test_matchString_question_mark(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/x?z'))
        self.assertEqual("__tes?/x?z\n", run('/bin/echo __tes?/x\?z'))

    def test_matchString_asterisk(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/*z'))
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/x*z'))
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/x*'))
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/x****yz'))
        self.assertEqual("__tes?/x*z\n", run('/bin/echo __tes?/x\*z'))

    def test_matchString_brackets(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__test/xyz\n", run('/bin/echo __tes?/x[ayql]z'))
        self.assertEqual("__tes?/x[abc]z\n", run('/bin/echo __tes?/x[abc]z'))
        self.assertEqual("__tes?/x[y]z\n", run('/bin/echo __tes?/x\[y]z'))

    def test_matchString_character_mismatch(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__tes?/xa*\n", run('/bin/echo __tes?/xa*'))

    def test_matchString_pattern_ends_before_string(self):
        os.makedirs("__test")
        writeFile("__test/xyz")
        self.assertEqual("__tes?/xy\n", run('/bin/echo __tes?/xy'))

    def test_matchBrackets(self):
        os.makedirs("__test")
        writeFile("__test/testFile")
        writeFile("__test/te[st")
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[qrstu]tFile'))
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[s-u]tFile'))
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[c-u]tFile'))
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[c-s]tFile'))
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[xa-el-u]tFile'))
        self.assertEqual("__test/te[^q-u]tFile\n",
                run('/bin/echo __test/te[^q-u]tFile'))
        self.assertEqual("__test/testFile\n",
                run('/bin/echo __test/te[^q-r]tFile'))
        self.assertEqual("__test/te[st\n",
                run('/bin/echo __test/te[[]*'))

class TestMain(unittest.TestCase):
    def tearDown(self):
        if os.path.exists("__test"):
            shutil.rmtree("__test")

    def test_loadArgs(self):
        os.makedirs("__test");
        writeFile("__test/script", "/bin/echo $0 $1 $2 '|' $# '|' $*")
        self.assertEqual("__test/script a b | 3 | a b c\n",
                runWithArgs("__test/script", "a", "b", "c"))
        self.assertEqual("0 | |\n",
                run("/bin/echo $# '|' $0 $1 '|' $*"))

    def test_main_c_option_basics(self):
        self.assertEqual("foo bar\n", runWithArgs("-c", "/bin/echo foo bar"))

    def test_main_c_option_missing_arg(self):
        self.assertIn("option requires an argument", runWithArgs("-c"))
        self.assertEqual(2, status)

    def test_main_c_option_exit_code(self):
        self.assertEqual("", runWithArgs("-c", "exit 44"))
        self.assertEqual(44, status)

    def test_main_script_file_basics(self):
        os.makedirs("__test");
        writeFile("__test/script", "/bin/echo 'foo\nbar'\n/bin/echo second command")
        self.assertEqual("foo\nbar\nsecond command\n",
                runWithArgs("__test/script"))

    def test_main_script_file_nonexistent_file(self):
        self.assertIn("_bogus_/xyzzy: No such file or directory\n",
                runWithArgs("_bogus_/xyzzy"))
        self.assertEqual(127, status)

    def test_main_script_file_exit_status(self):
        os.makedirs("__test");
        writeFile("__test/script", "/bin/echo command output\nexit 32\n    \n")
        self.assertEqual("command output\n", runWithArgs("__test/script"))
        self.assertEqual(32, status)

    def test_main_script_file_command_line_args(self):
        os.makedirs("__test");
        writeFile("__test/script", "/bin/echo $# '|' $0 $1 '|' $*\n")
        self.assertEqual("4 | __test/script a b | a b c d e\n",
                runWithArgs("__test/script", "a b", "c", "d", "e"))
        self.assertEqual(0, status)

class TestParser(unittest.TestCase):
    def tearDown(self):
        if os.path.exists("__test"):
            shutil.rmtree("__test")

    def test_eval_basics(self):
        self.assertEqual("foo abc $y\n", run("x=foo; /bin/echo $x abc '$y'"))

    def test_eval_input_file_subs(self):
        os.makedirs("__test")
        writeFile("__test/foo", "abcde");
        self.assertEqual("abcde", run("x=foo; /bin/cat <__test/$x"))

    def test_eval_output_file_subs(self):
        os.makedirs("__test")
        self.assertEqual("", run("x=foo; /bin/echo foo bar >__test/$x"))
        self.assertEqual("foo bar\n", readFile("__test/foo"))

    def test_eval_errors(self):
        self.assertIn("unexpected EOF while looking for matching `''",
                run("/bin/echo 'a b c"))
        self.assertIn("${}: bad substitution", run("/bin/echo a b ${}"))
        output = run("/bin/echo start; echo ${}")
        self.assertIn("${}: bad substitution", output)
        self.assertIn("start\n", output)

    def test_doSubs_tildes_first(self):
        home = run("/bin/echo ~")[:-1]
        self.assertNotIn("~", home)
        self.assertEqual("%s ~ ~ ~ ~\n"% (home),
                run("x='~ ~'; /bin/echo ~ $x \"~\" '~'"))

    def test_doSubs_variables(self):
        self.assertEqual("Word 1: foo\nWord 2: a\n"
                + "Word 3: b\nWord 4: a b\n",
                run("x=foo; y='a b'; ./words.py $x $y \"$y\""))
        self.assertEqual("a\nb\n", run("x='a\nb'; /bin/echo \"$x\""))
        self.assertEqual("Word 1: a\nWord 2: b\nWord 3: c\n",
                run("x='a   b   c  '; ./words.py $x"))
        self.assertEqual("", run("x=''; ./words.py $x $x"))
        self.assertEqual("Word 1: .\nWord 2: a\nWord 3: b\nWord 4: .\n",
                run("x=' a b '; ./words.py .$x."))

    def test_doSubs_commands(self):
        self.assertEqual("abc\n", run("x=abc; /bin/echo `/bin/echo $x`"))
        self.assertEqual("$abc\n", run("x='$abc'; /bin/echo `/bin/echo $x`"))

    def test_doSubs_backslashes(self):
        self.assertEqual("$x \"$x\n", run("x=abc; /bin/echo \$x \"\\\"\\$x\""))

    def test_doSubs_double_quotes(self):
        self.assertEqual("Word 1: a x y z b\n",
                run('./words.py "a `/bin/echo x y z` b"'))
        self.assertEqual("Word 1: \nWord 2: \nWord 3: \n",
                run("x=\"\"; ./words.py $x\"\" \"\" \"$x\""))

    def test_doSubs_single_quotes(self):
        self.assertEqual("Word 1: a $x `echo foo`\n",
                run("x=abc; ./words.py 'a $x `echo foo`'"))
        self.assertEqual("Word 1: \nWord 2: \nWord 3: \n",
                run("x=''; ./words.py $x'' ''$x ''"))

    def test_doSubs_path_expansion(self):
        os.makedirs("__test")
        writeFile("__test/a.c")
        writeFile("__test/b.c")
        result = run("x='*.c'; /bin/echo __test/*.c")
        self.assertIn("__test/a.c", result)
        self.assertIn("__test/b.c", result)
        result = run("x='*.c'; /bin/echo __test/$x")
        self.assertIn("__test/a.c", result)
        self.assertIn("__test/b.c", result)
        self.assertEqual("__test/*.c\n",
                run("x='*.c'; /bin/echo \"__test/*.c\""))

    def test_parse_leading_separators(self):
        self.assertEqual("a b c\n", run("/bin/echo a   b   c  "))

    def test_parse_single_quotes(self):
        self.assertEqual("Word 1: xyz \\\nWord 2: abc\n",
                run("./words.py 'xyz \\' abc"))

    def test_parse_double_quotes(self):
        self.assertEqual("Word 1: a b c\n",
                run("./words.py \"a b c\""))
        self.assertEqual("Word 1: a b\n",
                run("x='a b'; ./words.py \"$x\""))
        self.assertEqual("Word 1: foo bar\n",
                run("./words.py \"`/bin/echo foo bar`\""))
        self.assertEqual("Word 1: a\\b$x\n",
                run("x=abc; ./words.py \"a\\b\$x\""))
        self.assertEqual("\"\n", run("/bin/echo '\"'"))

    def test_parse_variables(self):
        self.assertEqual("abcyyy\n", run("xxx=abc; /bin/echo ${xxx}yyy"))
        self.assertIn("unexpected EOF while looking for `}'",
                run("/bin/echo ${xxx"))
        self.assertEqual("abc.z\n", run("x0yz4=abc; /bin/echo $x0yz4.z"))
        self.assertEqual("a55b\n", run("/bin/bash -c 'exit 55'; /bin/echo a$?b"))
        self.assertIn("${}: bad substitution\n", run("/bin/echo ${}"))
        self.assertEqual("Word 1: a\nb\nc\n",
                run("x='a\nb\nc'; ./words.py \"$x\""))
        self.assertEqual("$ $x $\n", run("x0yz4=abc; /bin/echo $ \"$\"x $"))
        self.assertEqual("arg12xy\n",
                runWithArgs("-c", "echo $12xy", "arg0", "arg1", "arg2"))

    def test_parse_commands_basics(self):
        self.assertEqual("Word 1: $y\n",
                run("x='$y'; y=abc; /bin/echo `./words.py $x`"))
        self.assertEqual("Word 1: a b c Word 2: x y\n",
                run("/bin/echo `./words.py \"a b c\" 'x y'`"))
        self.assertEqual("`\n", run("/bin/echo '`'"))

    def test_parse_commands_no_word_breaks(self):
        self.assertEqual("Word 1: a b c\n",
                run("./words.py \"`/bin/echo a b c`\""))

    def test_parse_commands_errors(self):
        self.assertIn("Unexpected EOF while looking for matching ``'",
                run("/bin/echo `foo bar"))

    def test_parse_backslashes(self):
        self.assertEqual("aa$x`echo foo`\n",
                run("x=99; /bin/echo a\\a\\$x\\`echo foo\\`"))
        self.assertIn("Unexpected EOF while parsing backslash",
                run("/bin/echo \\"))
        self.assertIn("Unexpected EOF after backslash-newline",
                run("/bin/echo \\\n"))
        self.assertEqual("Word 1: axyz\n", run("./words.py a\\\nxyz"))

    def test_parse_backslashes_in_quotes(self):
        self.assertEqual("a$x`b\"c\\d\ne\\a\n",
                run("x=99; /bin/echo \"a\\$x\\`b\\\"c\\\\d\\\ne\\a\""))

    def test_parse_backslashes_meaningless(self):
        self.assertEqual("a\\b\n", run("/bin/echo 'a\\b'"))

    def test_split_basics(self):
        os.makedirs("__test")
        self.assertEqual("abc def\n", run("/bin/echo abc def"))
        self.assertEqual("", run("/bin/echo abc def > __test/foo"))
        self.assertEqual("", run("/bin/echo abc def>__test/foo"))
        self.assertEqual("abc def\n", readFile("__test/foo"))
        self.assertEqual("abc def\n", run("/bin/cat < __test/foo"))
        self.assertEqual("abc def\n", run("/bin/cat<__test/foo"))

    def test_split_empty_first_word(self):
        os.makedirs("__test")
        self.assertEqual("", run("> __test/foo /bin/echo abc"))
        self.assertEqual("abc\n", readFile("__test/foo"))

    def test_split_missing_input_file(self):
        os.makedirs("__test")
        self.assertEqual("-clash: no file given for input redirection\n",
                run("/bin/echo abc < >__test/foo"))

    def test_split_missing_output_file(self):
        os.makedirs("__test")
        self.assertEqual("-clash: no file given for output redirection\n",
                run("/bin/echo abc >;"))

    def test_split_pipeline(self):
        os.makedirs("__test")
        self.assertEqual("abc def\n", run("/bin/echo abc def | /bin/cat"))
        self.assertEqual("abc def\n", run("/bin/echo abc def|/bin/cat"))
        self.assertEqual("     1\t     1\tabc def\n",
                run("/bin/echo abc def | /bin/cat -n | /bin/cat -n"))

    def test_split_multiple_pipelines(self):
        os.makedirs("__test")
        self.assertEqual("xyz\n",
                run("/bin/echo abc>__test/out1;/bin/echo def > __test/out2;"
                        + "/bin/echo xyz"))
        self.assertEqual("abc\n", readFile("__test/out1"))
        self.assertEqual("def\n", readFile("__test/out2"))
        self.assertEqual("xyz\n",
                run("/bin/echo abc>__test/out1;/bin/echo def > __test/out2 ;"
                        + "/bin/echo xyz"))
        self.assertEqual("abc\n", readFile("__test/out1"))
        self.assertEqual("def\n", readFile("__test/out2"))

    def test_breakAndAppend(self):
        self.assertEqual("Word 1: .abc\nWord 2: def\nWord 3: x\nWord 4: y.\n",
                run("x='abc def\tx\ny'; ./words.py .$x."))
        self.assertEqual("Word 1: .\nWord 2: a\nWord 3: b\nWord 4: .\n",
                run("x='  \t\n a b \t\n'; ./words.py .$x."))

class TestPipeline(unittest.TestCase):
    # Much of the code in this class was already tested by other
    # tests, such as those for Parser.
    def tearDown(self):
        if os.path.exists("__test"):
            shutil.rmtree("__test")

    def test_rebuildPathMap_basics(self):
        os.makedirs("__test")
        os.makedirs("__test/child")
        writeFile("__test/a", "#!/bin/sh\n/bin/echo __test/a")
        os.chmod("__test/a", 0o777)
        writeFile("__test/b", "#!/bin/sh\n/bin/echo __test/b")
        os.chmod("__test/b", 0o777)
        writeFile("__test/child/a", "#!/bin/sh\n/bin/echo __test/child/a")
        os.chmod("__test/child/a", 0o777)
        writeFile("__test/child/b", "#!/bin/sh\n/bin/echo __test/child/b")
        self.assertEqual("__test/child/a\n__test/b\n",
                run("PATH=\"`/bin/pwd`/__test/child:`/bin/pwd`/__test\"; a; b"))

    def test_rebuildPathMap_default(self):
        self.assertEqual("a b\n", run("unset PATH; echo a b"))

    def test_run_redirection_basics(self):
        os.makedirs("__test")
        self.assertEqual("a b c\n",
                run("/bin/echo a b c > __test/foo; /bin/cat __test/foo"))
        self.assertEqual("a b c\n", run("/bin/cat < __test/foo"))

    def test_run_ambiguous_input_redirection(self):
        self.assertIn("Ambiguous input redirection",
                run("x='a b'; /bin/cat <$x"))

    def test_run_bad_input_file(self):
        os.makedirs("__test")
        self.assertIn("No such file or directory",
                run("cat < __test/bogus"))

    def test_run_ambiguous_output_redirection(self):
        self.assertIn("Ambiguous output redirection",
                run("x='a b'; /bin/echo foo bar >$x"))

    def test_run_pipeline(self):
        self.assertIn("x y z\n", run("/bin/echo x y z | cat"))

    def test_run_bad_output_file(self):
        os.makedirs("__test")
        self.assertIn("No such file or directory",
                run("/bin/echo abc > __test/_bogus/xyz"))

    def test_run_rebuild_path_cache_to_discover_new_file(self):
        os.makedirs("__test")
        writeFile("__test/x", "#!/bin/sh\n/bin/echo __test/x")
        self.assertIn(" x: command not found\n__test/x",
                run("PATH=\"/bin:`pwd`/__test\"; x; chmod +x __test/x; x"))

    def test_run_rebuild_path_cache_path_changed(self):
        os.makedirs("__test")
        writeFile("__test/x", "#!/bin/sh\n/bin/echo __test/x")
        os.chmod("__test/x", 0o777)
        self.assertIn(" x: command not found\n__test/x",
                run("x; PATH=\"/bin:`pwd`/__test\"; x"))

    def test_run_no_such_executable(self):
        os.makedirs("__test")
        self.assertIn("No such file or directory", run("__test/bogus foo bar"))
        self.assertIn("No such file or directory",
                run("__test/bogus foo bar | /bin/echo foo"))
        self.assertNotIn("No such file or directory",
                run("__test/bogus foo bar |& /bin/echo foo"))
        self.assertEqual("",
                run("__test/bogus foo bar |& /bin/cat > __test/out"))
        self.assertIn("No such file or directory",
                readFile("__test/out"))

    def test_run_set_status_variable(self):
        os.makedirs("__test")
        self.assertEqual("0\n", run("/bin/true; /bin/echo $?"))
        self.assertEqual("44\n", run("/bin/bash -c \"exit 44\"; /bin/echo $?"))

class TestVariables(unittest.TestCase):
    def test_set(self):
        self.assertEqual("50 100\n", run("x=99; y=100; x=50; /bin/echo $x $y"))
        self.assertIn("x=100",
                run("x=99; export x; /bin/echo foo; x=100; /usr/bin/printenv"))

    def test_setFromEnviron(self):
        self.assertNotEqual("\n", run("/bin/echo $HOME"))
        self.assertNotEqual("\n", run("/bin/echo $SHELL"))
        self.assertIn("SHELL=xyzzy", run("SHELL=xyzzy; /usr/bin/printenv"))

    def test_set(self):
        self.assertEqual(".99. ..\n", run("x=99; /bin/echo .$x. .$y."))

    def test_getEnviron(self):
        result = run("x=99; export x; y=100; /usr/bin/printenv")
        self.assertIn("x=99", result)
        self.assertNotIn("y=", result)
        self.assertIn("HOME=", result)
        self.assertIn("SHELL=", result)

    def test_unset(self):
        self.assertEqual("~99~\n~~\n",
                run("x=99; /bin/echo ~$x~; unset x; /bin/echo ~$x~"))
        result = run("x=99; export x; echo x:$x; unset x; /usr/bin/printenv")
        self.assertIn("x:99", result);
        self.assertNotIn("x=99", result);

    def test_markExported(self):
        self.assertNotIn("x=99", run("x=99; /usr/bin/printenv"))
        self.assertIn("x=99", run("x=99; /bin/echo foo bar; export x; "
                "/usr/bin/printenv"))

if __name__ == '__main__':
  unittest.main()