require 'yaml'

# TODO:
#  Add better reporting of sub-tests
#  Add way to recieve "args" and selectively run tests based on the arguments

ARGV.each do |arg|
    puts arg
end

test_num = 0
YAML.load_file('./_test/tests.yaml').each do |name, tests|
    puts "Running Test #{test_num}: #{name} - Testing #{tests['desc']}"
    tests['tests'].each do |test|
        in_files = test['files'].map { |file| "./_test/#{file}" }.join(' ')
        out_file = "_test/tmp/" + (test.key?('out') ? test['out'] : 'out.exe')
        args = test['args'].join(' ')
        speroc_str = "./_test/speroc #{in_files} -o #{out_file} #{args}"

        system(speroc_str)
        system("./#{out_file}")
        puts $?.exitstatus == test['value']
    end
    test_num = test_num + 1
end