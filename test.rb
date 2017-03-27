require 'yaml'

# TODO:
#  Improve display of testing output
#  Move test group naming to lowercase
#    Implement more useful filtering of test cases (use 'tag' field)

# Catch output (of individual tests)
# {testloc}: {status}:
#   {test}
# with {failure}



# TODO (from _todo.txt)
# improve reporting/handling of tests in the ruby script
# implement ability to selectively run tests from command line tags
# work on improving the test yaml organization

# Eventual TODO (Can't implement yet)
#  Figure out how to report on compilation errors
#    First need speroc to report compilation errors

test_num = 0
YAML.load_file('./_test/tests.yaml').each do |name, tests|
    if ARGV.size > 0 && !ARGV.include?(name) then
        next
    end

    # Test group setup
    num_correct = 0
    puts "Running Test #{test_num}: #{name} - Testing #{tests['desc']}"

    # Run all the tests in the test group
    tests['tests'].each do |test|
        # Collect information for calling the compiler from the test specification
        in_files = test['files'].map { |file| "./_test/#{file}" }.join(' ')
        out_file = "_test/tmp/" + (test.key?('out') ? test['out'] : 'out.exe')
        args = test['args'].join(' ')
        speroc_cmd = "./_test/speroc #{in_files} -o #{out_file} #{args}"


        # Compile the program and catch any compilation errors
        IO.popen(speroc_cmd) do |io|
            # Capture any compilation errors here
            io.read
            io.close

            # If compilation failed, display errors
            if $?.exitstatus != 0 then
                puts "Test #{test_num} Failed:   Compilation Error"
            end
        end

        # If compilation succeeded, run the program
        if $?.to_i == 0 then
            # Run the created executable
            IO.popen("./#{out_file}") do |io|
                io.read
                io.close
            end

            # Test whether it worked or not
            if $?.exitstatus != test['value'] then
                puts "Test #{test_num} Failed:   Expected value '#{test['value']}' - Got value '#{$?.to_i}'"
            else
                puts "Test #{test_num} Succeded with return '#{$?.exitstatus}'"
                num_correct = num_correct + 1
            end
        end

        # Update status for end output
        test_num = test_num + 1
    end

    # Report status of test programs
    num_tests = tests['tests'].size
    num_failed = num_tests - num_correct
    puts " -Report for Test Group #{name}: Succeed: #{num_correct}/#{num_tests} - Failed: #{num_failed}\n\n"
end

=begin
{test_group}
===============
{individual}
...
===============
test cases: {total} | {failed} failed
assertions: {total} | {failed} failed
=end