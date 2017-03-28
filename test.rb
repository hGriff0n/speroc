require 'yaml'
require 'optparse'

# TODO:
#  Switch over completely to the new yaml organization
#    asm_file
#    output
#    exec
#    :plus any other things I need to implement
#  Work on the displaying of the test cases (I don't quite like the look)
#  Collect all output from the compilation and execution steps in an array
#  Simplify the implementation of the runner


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


    # Test group setup
    num_correct = 0
    panic_pct = tests.key?('panic') ? tests['panic'] : 1


    # Initial Test Group Output
    puts "Running Test Group \"#{name.capitalize}\""
    puts "  - Testing #{tests['desc']}\n"
    puts "======================================================="


    # Run all the tests in the test group
    tests['tests'].each do |test|
        can_run = true

        # Test Setup (setting default variables/etc)
        compile = test.key?('compile') ? test['compile']
            : { :norun => true, 'files' => [], 'args' => [], 'out' => test['exec'] }
        success_metric = test.key?('return') ? 0 : test.key?('asm_file') ? 1 : test.key?('output') ? 2 : 3
        simple_cmd = exec_file = compile['out']
        out_file = "_test/tmp/#{exec_file}"


        # Perform compilation if the specification requires it
        if !compile[:norun] then 

            # Generate strings for correctly calling the compiler from the runner
            in_files = compile['files'].map { |file| "./_test/#{file}" }.join(' ')
            args = compile['args'].join(' ')
            speroc_cmd = "./_test/speroc #{in_files} -o #{out_file} #{args}"

            # Generate a smaller string to simplify test print messages
            simple_cmd = "speroc #{compile['files'].join(' ')} -o #{exec_file} #{args}"


            # Compile the program and catch any compilation errors
            IO.popen(speroc_cmd) do |io|
                # Capture any compilation errors and close the process
                compile['out'] = io.readlines
                io.close

                # If compilation failed, display errors and skip to the next test
                can_run = $?.exitstatus == 0
                if !can_run then
                    if !test['fail'] then
                        puts " [#{test_num}]: #{simple_cmd}"
                        puts "    Compilation failed with errorstatus #{$?.exitstatus}"
                        # TODO: Report compilation messages
                        puts " --------------------------------------------------"
                    else
                        num_correct += 1
                    end

                elsif test['fail'] then
                    puts " [#{test_num}]: #{simple_cmd}"
                    puts "    Compilation succeeded when it should have failed"
                end
            end
        end

        # If compilation succeeded, run the program
        if can_run || compile[:norun] then

            # Run the created executable
            IO.popen("./#{out_file}") do |io|
                execute_out = io.readlines
                io.close

                # Test whether it worked or not
                if $?.exitstatus != test['return'] && !test['fail'] then
                    puts " [#{test_num}]: #{simple_cmd}"
                    puts "    Execution failed with incorrect return value"
                    puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                    puts " --------------------------------------------------"
                elsif test['fail'] then
                    puts " [#{test_num}]: #{simple_cmd}"
                    puts "    Execution succeeded when it should have failed"
                    puts "     Expected: #{test['return']}  =>  Actual: #{$?.exitstatus}"
                    puts " --------------------------------------------------"
                else
                    num_correct += 1
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
    if num_tests != 0 then
        puts "   #{num_tests - num_correct} cases out of #{num_tests} possible failed | #{num_correct} passed"
    end
    if (1 - pass_pct) > panic_pct then
        puts "   Panic: #{(1 - panic_pct) * 100}% of tests must pass out of this group"
        puts "     Stopping due to #{(1 - pass_pct) * 100}% test failure rate"
        puts "=======================================================\n\n"
        break
    end
    puts "=======================================================\n\n"
end