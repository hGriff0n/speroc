require 'yaml'
require 'optparse'

# TODO:
#  Switch over to the new yaml organization
#    Make sure the new organization is covered
#  Work on the displaying of the test cases (I don't quite like the look)
#  Collect all output from the compilation and execution steps in an array

# Parse out the command line arguments
options = {
    # provide default values for some commands
    :file => './_test/tests.yaml',
    :ignore => false,
}
OptionParser.new do |opt|
    opt.on("-f", "--file [FILE]") { |o| options[:file] = o }
    opt.on("-i", "--ignore") { |o| options[:ignore] = o }
end.parse!

ARGV.map! {|s| s.downcase}

# Run the actual test framework
test_num = 0
YAML.load_file(options[:file]).each do |name, tests|
    is_arg = (tests['tags'] & ARGV)
    next if options[:ignore] && is_arg
    next if !options[:ignore] && !is_arg

    # Test group setup
    num_correct = 0
    puts "Running Test Group \"#{name.capitalize}\""
    puts "  - Testing #{tests['desc']}\n"
    puts "======================================================="

    # Run all the tests in the test group
    tests['tests'].each do |test|
        # Collect information for calling the compiler from the test specification
        in_files = test['files'].map { |file| "./_test/#{file}" }.join(' ')
        simp_out = test.key?('out') ? test['out'] : 'out.exe'
        out_file = "_test/tmp/#{simp_out}"
        args = test['args'].join(' ')
        simple_cmd = "speroc #{test['files'].join(' ')} -o #{simp_out} #{args}"
        speroc_cmd = "./_test/speroc #{in_files} -o #{out_file} #{args}"


        # Compile the program and catch any compilation errors
        IO.popen(speroc_cmd) do |io|
            # Capture any compilation errors here
            compilation_out = io.readlines
            io.close

            # If compilation failed, display errors
            if $?.exitstatus != 0 then
                puts " [#{test_num}]: #{simple_cmd}"
                puts "    Compilation failed with errorstatus #{$?.exitstatus}"
                # TODO: Report compilation messages
                puts " --------------------------------------------------"
            end
        end

        # If compilation succeeded, run the program
        if $?.to_i == 0 then
            # Run the created executable
            IO.popen("./#{out_file}") do |io|
                io.read
                io.close

                # Test whether it worked or not
                if $?.exitstatus != test['value'] then
                    puts " [#{test_num}]: #{simple_cmd}"
                    puts "    Execution failed with incorrect return value"
                    puts "     Expected: #{test['value']}, Actual: #{$?.exitstatus}"
                    puts " --------------------------------------------------"
                else
                    # puts "  Test #{test_num} Succeded with return '#{$?.exitstatus}'"
                    num_correct = num_correct + 1
                end
            end
        end

        # Update status for end output
        test_num = test_num + 1
    end

    # Report status of test programs
    num_tests = tests['tests'].size
    puts "  - Report for Test Group \"#{name.capitalize}\""
    if num_tests != 0 then
        puts "   #{num_tests - num_correct} casses out of #{num_tests} possible failed | #{num_correct / num_tests}% passed"
    end
    puts "=======================================================\n\n"
    # puts " -Report for Test Group #{name}: Succeed: #{num_correct}/#{num_tests} - Failed: #{num_failed}\n\n"
end

# Catch output (of individual tests)
# {testloc}: {status}:
#   {test}
# with {failure}

=begin
{test_group}
===============
{individual}
...
===============
test cases: {total} | {failed} failed
assertions: {total} | {failed} failed
=end