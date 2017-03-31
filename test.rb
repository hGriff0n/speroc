require 'yaml'
require 'optparse'

# TODO:
#  Work on the displaying of the test cases (I don't quite like the look)
#  Add in the ability to specify directory from the yaml
#  Improve yaml documentation



# Global defines for the project
SUCCESS = { :ret => 0, :asm => 1, :out => 2, :na => 3 }

# Generate default values for compilation and execution commands
def compile_defaults(run)
    if run.key?('compile')
        compile = run['compile']
        if !compile.key?('out') then compile['out'] = "out.exe" end
        if !compile.key?('args') then compile['args'] = [] end
        compile['exec'] = "_test/tmp/#{run['exec']}"
        compile
    else
        { :norun => true, 'files' => [], 'args' => [], 'out' => run['exec'], 'fail' => false }
    end
end



# Parse out the command line arguments
options = {
    :file => './_test/tests.yaml',
    :ignore => false,
    :dir => "_test",
    :out_dir => "tmp"
}
OptionParser.new do |opt|
    opt.on("-f", "--file [FILE]") { |f| options[:file] = f }
    opt.on("-i", "--ignore") { |i| options[:ignore] = i }
    opt.on("-d", "--dir [DIR]") { |d| options[:dir] = d }
    opt.on("-o", "--out [DIR]") { |d| options[:out_dir] = d }
end.parse!

# convert the remaining args to lowercase (same as tags)
ARGV.map! {|s| s.downcase}


# Run the actual test framework
test_num = 0
yaml = YAML.load_file(options[:file])
yaml.each do |name, tests|

    # Skip this test group if specified
    in_cmd_tags = (tests['tags'] & ARGV)
    next if options[:ignore] && in_cmd_tags
    next if !options[:ignore] && !in_cmd_tags


    # Setup group defaults and counters
    num_correct = num_tests = 0
    group_desc = tests.key?('desc') ? tests['desc'] : ""
    panic_pct = tests.key?('panic') ? tests['panic'] : 1


    # Initial Test Group Output
    puts "Running Test Group \"#{name.capitalize}\""
    puts "  - Testing #{group_desc}\n"
    puts "======================================================="

    
    # Run over all the unique executables in the test group
    tests['runs'].each_with_index do |run, idx|

        # Setup run defaults and other variables
        run_desc = run.key?('desc') ? run['desc'] : ""
        compile = compile_defaults(run)
        simple_cmd = run['exec']
        run_tests = 0


        # Perform compilation if the run requires it (likely)
        if ! compile[:norun]
            
            # Generate strings for correctly calling the compiler from the runner
            in_files = compile['files'].map { |file| "./_test/#{file}" }.join(' ')
            args = compile['args'].join(' ')
            speroc_cmd = "./_test/speroc #{in_files} -o #{compile['exec']} #{args}"

            # Generate a smaller string to simplify test print messages
            simple_cmd = "speroc #{compile['files'].join(' ')} -o #{run['exec']} #{args}"


            # Compile the program and report errors (if any)
            IO.popen(speroc_cmd) do |io|

                # Capture any compilation errors and close the process
                compile['out'] = io.readlines
                io.close

                # Store compilation success and mark test number
                compile[:success] = $?.exitstatus == 0
                run_tests += 1


                # If compilation failed when it should have succeeded
                if !compile[:success] && !compile['fail']
                    puts " [#{idx}:#{run_tests}]: #{simple_cmd}"
                    puts "    Compilation failed with errorstatus #{$?.exitstatus}"
                    # TODO: Report compilation messages
                    puts " --------------------------------------------------"

                # If compilation succeeded when it should have failed
                elsif compile[:success] && compile['fail']
                    puts " [#{idx}:#{run_tests}]: #{simple_cmd}"
                    puts "    Compilation succeeded when it should have failed"

                # TODO: Test against produced assembly if required
                elsif compile[:success] && compile.key?('asm')
                    run_tests += 1
                    num_correct += 1

                    FileUtils.identical?(compile['asm'], "out.s")
                    can_run = false

                else
                    num_correct += 1
                end

            end
        end


        # Run over all tests if compilation succeeded (or the executable already existed)
        run['tests'].each do |test|

            # Setup test defaults and variables
            if !test.key?('args') then test['args'] = [] end
            # test = set_test_defaults(test)


            # Run the exeuctable with the test specific arguments
            IO.popen("./#{compile['exec']} #{test['args'].join(' ')}") do |io|
                exec_out = io.readlines
                io.close


                # Test whether the return value matches (if required)
                if test.key?('return')
                    run_tests += 1

                    if $?.exitstatus != test['return'] && !test['fail']
                        puts " [#{idx}:#{run_tests}]: #{simple_cmd}"
                        puts "    Execution failed with incorrect return value"
                        puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                        puts " --------------------------------------------------"
                    elsif test['fail']
                        puts " [#{idx}:#{run_tests}]: #{simple_cmd}"
                        puts "    Execution succeeded when it should have failed"
                        puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end
                end

                
                # Test whether the program's output matches expectations (if required)
                if test.key?('output')
                    run_tests += 1
                    files_match = true

                    # Compare every line of the file
                    File.readlines(test['output']).each_with_index do |line, lineno|
                        if exec_out[lineno] != line
                            files_match = false
                            break
                        end
                    end


                    if !files_match && !test['fail']
                        puts " [#{run_n}:#{run_tests}]: #{simple_cmd}"
                        puts "    Execution failed to match expected output"
                        puts "    See file real_#{test['out']} for actual output"
                        puts " --------------------------------------------------"

                        File.open("read_#{test['out']}", 'w') { |f| execute_out.each { |line| f.puts(line) } }
                    elsif test['fail']
                        puts " [#{run_n}:#{run_tests}]: #{simple_cmd}"
                        puts "    Execution matched expected output when it should have failed"
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end
                end
            end

        end if compile[:success] || compile[:norun]

        # Update runner variables and state
        num_tests += run_tests

    end


    # Report status of test programs and possibly panic if required
    pass_pct = num_correct / (num_tests * 1.0)
    puts "  - Report for Test Group \"#{name.capitalize}\""
    if num_tests != 0
        puts "   #{num_tests - num_correct} cases out of #{num_tests} possible failed | #{num_correct} passed"
    end
    if (1 - pass_pct) > panic_pct
        puts "   Panic: #{((1 - panic_pct) * 100).round(2)}% of tests must pass out of this group"
        puts "     Stopping due to #{((1 - pass_pct) * 100).round(2)}% test failure rate"
        puts "=======================================================\n\n"
        break
    end
    puts "=======================================================\n\n"
end
