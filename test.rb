require 'yaml'
require 'optparse'

# TODO:
#  Switch over completely to the new yaml organization
#    :plus any other things I need to implement
#  Work on the displaying of the test cases (I don't quite like the look)
#    How to control formatting of floats
#    Need the ability to differentiate between failures at runtime (currently displays compilation)
#  Simplify the implementation of the runner
#    Have "data" pointing to various compiler flags and other constants
#  Ensure that 'asm_file' forces the compiler to produce assembly code
#    Test the behavior of asm_file once the flags are implemented
#  Improve the way files are located from the test data (current approach will fail alot I feel)


# Global defines for the project
SUCCESS = { :ret => 0, :asm => 1, :out => 2, :na => 3 }

# Generate default values for compilation and execution commands
def get_test_compilation(test)
    if test.key?('compile')
        compile = test['compile']
        if !compile.key?('out') then compile['out'] = "out.exe" end
        if !compile.key?('args') then compile['args'] = [] end
        compile
    else
        { :norun => true, 'files' => [], 'args' => [], 'out' => test['exec'], 'fail' => false }
    end
end

def get_test_execution(test, compile)
    if test.key?('exec')
        exec = test['exec']
        if !exec.key?('fail') then exec['fail'] = false end
        if !exec.key?('file') then exec['file'] = "_test/tmp/#{compile['out']}" end
        if !exec.key?('args') then exec['args'] = [] end
        exec
    else
        { 'fail' => false, 'file' => compile['out'], 'args' => [] }
    end
end



# Parse out the command line arguments
options = {
    :file => './_test/tests.yaml',
    :ignore => false,
}
OptionParser.new do |opt|
    opt.on("-f", "--file [FILE]") { |o| options[:file] = o }
    opt.on("-i", "--ignore") { |o| options[:ignore] = o }
end.parse!

# convert the remaining args to lowercase (same as tags)
ARGV.map! {|s| s.downcase}


# Run the actual test framework
test_num = 0
YAML.load_file(options[:file]).each do |name, tests|
    # Skip this test group if specified
    is_arg = (tests['tags'] & ARGV)
    next if options[:ignore] && is_arg
    next if !options[:ignore] && !is_arg


    # Setup group defaults and counters
    num_correct = 0
    group_desc = tests.key?('desc') ? tests['desc'] : ""
    panic_pct = tests.key?('panic') ? tests['panic'] : 1


    # Initial Test Group Output
    puts "Running Test Group \"#{name.capitalize}\""
    puts "  - Testing #{group_desc}\n"
    puts "======================================================="


    # Run all the tests in the test group
    tests['tests'].each do |test|

        # Setup test defaults and variables
        can_run = true
        test_desc = test.key?('desc') ? test['desc'] : ""
        compile = get_test_compilation(test)
        exec = get_test_execution(test, compile)
        success_metric = test.key?('return') ? SUCCESS[:ret] : test.key?('asm_file') ? SUCCESS[:asm]
            : test.key?('output') ? SUCCESS[:out] : SUCCESS[:na]
        simple_cmd = compile['out']


        # Perform compilation if the specification requires it
        if !compile[:norun] 

            # Generate strings for correctly calling the compiler from the runner
            in_files = compile['files'].map { |file| "./_test/#{file}" }.join(' ')
            args = compile['args'].join(' ')
            speroc_cmd = "./_test/speroc #{in_files} -o #{exec['file']} #{args}"

            # Generate a smaller string to simplify test print messages
            simple_cmd = "speroc #{compile['files'].join(' ')} -o #{compile['out']} #{args}"


            # Compile the program and catch any compilation errors
            IO.popen(speroc_cmd) do |io|
                # Capture any compilation errors and close the process
                compile['out'] = io.readlines
                io.close

                # If compilation failed, display errors and skip to the next test
                can_run = $?.exitstatus == 0
                if !can_run
                    if !compile['fail']
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Compilation failed with errorstatus #{$?.exitstatus}"
                        # TODO: Report compilation messages
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end

                # If the compilation was supposed to fail
                elsif compile['fail']
                    puts " [#{test_num}]: #{simple_cmd}"
                    puts "    Compilation succeeded when it should have failed"

                # TODO: Test against the produced assembly if expected
                elsif success_metric == SUCCESS[:asm]
                    FileUtils.identical?(test['asm_file'], "./#{exec['file']}")
                    can_run = false
                end
            end
        end

        # If compilation succeeded, run the program
        if can_run || compile[:norun]

            # Run the created executable
            IO.popen("./#{exec['file']} #{exec['args'].join(' ')}") do |io|
                execute_out = io.readlines
                io.close

                case success_metric
                when SUCCESS[:ret]
                    # Test whether it worked or not
                    if $?.exitstatus != test['return'] && !exec['fail']
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Execution failed with incorrect return value"
                        puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                        puts " --------------------------------------------------"
                    elsif exec['fail']
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Execution succeeded when it should have failed"
                        puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end
                when SUCCESS[:out]
                    # Compare program output to the file in test['out']
                    files_match = true
                    File.readlines(test['out']).each_with_index do |line, lineno|
                        if execute_out[lineno] != line
                            files_match = false
                            break
                        end
                    end

                    # Test whether it worked or not
                    if !files_match && !exec['fail']
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Execution failed to match expected output"
                        puts "    See file real_#{test['out']} for actual output"
                        puts " --------------------------------------------------"

                        File.open("read_#{test['out']}", 'w') { |f| execute_out.each { |line| f.puts(line) } }

                    elsif exec['fail']
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Execution matched expected output when it should have failed"
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end
                else
                    puts " [#{test_num}]: Invalid success metric defined"
                end
            end
        end

        # Update status for end output
        test_num = test_num + 1
    end

    # Report status of test programs and possibly panic if required
    num_tests = tests['tests'].size
    pass_pct = num_correct / (num_tests * 1.0)
    puts "  - Report for Test Group \"#{name.capitalize}\""
    if num_tests != 0
        puts "   #{num_tests - num_correct} cases out of #{num_tests} possible failed | #{num_correct} passed"
    end
    if (1 - pass_pct) > panic_pct
        puts "   Panic: #{(1 - panic_pct) * 100}% of tests must pass out of this group"
        puts "     Stopping due to #{(1 - pass_pct) * 100}% test failure rate"
        puts "=======================================================\n\n"
        break
    end
    puts "=======================================================\n\n"
end