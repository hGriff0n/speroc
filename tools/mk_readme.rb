# Simple ruby script that constructs the readme file from various sources
# The primary purpose of this script is to automate the process of updating semantic versioning
# However, this can also be extended in the future to perform various tasks

require_relative 'utils'
require 'yaml'

# TODO: Move this file into the 'tools' directory

yaml_file = "./docs/version.yaml"
ver = YAML.load_file(yaml_file)
File.open("./README.md", "w") do |readme|
    # Semantic versioning (TODO: Determine how to autoupdate)
    readme.puts "speroc ver #{ver['major']}.#{ver['minor']}.#{ver['patch']} - The reference compiler for the spero language"

    # Move over the custom readme information into the public readme
    readme.puts ""
    File.open("./docs/_readme.md", "r") do |file|
        file.each_line do |line|
            readme.puts line
        end
    end

    # Collect information about files in the project (TODO: See if I can add in more stats)
    stats = SourceStats.new(["incl", "src", "tools", "main.cpp"])
    num_lines = stats.exts.reduce(0) do |acc, elem|
        acc + elem[1][:sloc]
    end

    # Output the source file information
    readme.puts "\nProject Info:"
    readme.puts "\n    size: #{num_lines} sloc"
    readme.print "    files: "
    count = 0
    exts = stats.exts.select { |_, data| data[:files] != 0 }
    exts.each do |key, data|
        readme.print "#{data[:files]} #{key}"
        count += 1
        readme.print ", " unless count == exts.length
    end
    readme.puts "\n\n"


    # TODO: Other stuff that I can possibly add in


end
