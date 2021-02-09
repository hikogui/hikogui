#!/usr/bin/env python3

import subprocess
import sys
import re
import datetime

copyright_reference = [
    "Distributed under the Boost Software License, Version 1.0.",
    "(See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)"
]

name_aliases = {
    "Tjienta Vara": "Take Vos"
}

class YearRange (object):
    def __init__(self, year):
        self.first_year = year
        self.last_year = year

    def __str__(self):
        if self.first_year == self.last_year:
            return "{}".format(self.first_year)
        else:
            return "{}-{}".format(self.first_year, self.last_year)

    def add_year(self, new_year):
        if new_year < self.last_year:
            raise RuntimeError("Trying to add years out of order")
        elif new_year <= self.last_year + 1:
            self.last_year = new_year
            return True
        else:
            return False

def format_years(years):
    year_ranges = []
    for year in sorted(list(years)):
        if len(year_ranges) == 0 or not year_ranges[-1].add_year(year):
            year_ranges.append(YearRange(year))
    
    return ", ".join(str(x) for x in year_ranges)

class Commit (object):
    def __init__(self):
        self.nr_lines = 0
        self.author = None
        self.years = set()

    def add_year(self, new_year):
        self.years.add(new_year)

class Contributor (object):
    def __init__(self, author):
        self.author = author
        self.years = set()

    def __cmp__(self, other):
        return cmp(self.author, other.author)

    def __str__(self):
        return "Copyright {} {}.".format(self.author, format_years(self.years))

    def add_years(self, new_years):
        self.years = self.years.union(new_years)


def git_blame_run(filename):
    result = subprocess.run(["git", "blame",  "-p", "--", filename], capture_output=True)
    if result.returncode != 0:
        raise RuntimeError("git blame -p -- '{}' failed.".format(filename))

    return result.stdout.decode("UTF-8")

def git_blame(filename):
    commits = {}

    commit = None
    for line in git_blame_run(filename).split("\n"):
        if line == "":
            break

        if commit is None:
            parts = line.split(" ")

            if 3 <= len(parts) <= 4:
                commit_sha = parts[0]
                commit = commits.setdefault(commit_sha, Commit())

            else:
                raise RuntimeError("Unexpected line where sha is expected: {}".format(line))

        elif line[0] == "\t":
            commit.nr_lines += 1
            commit = None

        elif line.startswith("author "):
            global name_aliases
            author = line[7:]
            commit.author = name_aliases.get(author, author)

        elif line.startswith("author-time "):
            posix_timestamp = int(line[12:])
            date = datetime.date.fromtimestamp(posix_timestamp)
            commit.add_year(date.year)

    return commits.values()


def extract_contributors(commits):
    contributors = {}

    for commit in commits:
        if commit.nr_lines >= 10:
            author = commit.author
            contributor = contributors.setdefault(author, Contributor(author))
            contributor.add_years(commit.years)

    return sorted(contributors.values())

def replace_copyright_detail(filename, contributors, prefix):
    global copyright_reference

    lines = open(filename, "rb").read().decode("UTF-8").split("\n")

    # Remove the comments at the start of the file.
    while lines and lines[0].startswith(prefix):
        del lines[0]

    if not lines:
        return

    # Insert contributors and copyright reference to the start.
    suffix = "\r" if lines[0].endswith("\r") else ""
    new_lines = []
    for contributor in contributors:
        new_lines.append("{}{}{}".format(prefix, contributor, suffix))
    for line in copyright_reference:
        new_lines.append("{}{}{}".format(prefix, line, suffix))

    lines = new_lines + lines
    text = "\n".join(lines)

    open(filename, "wb").write(text.encode("UTF-8"))

def replace_copyright(filename, contributors):
    if filename.endswith(".cpp") or filename.endswith(".hpp"):
        replace_copyright_detail(filename, contributors, "// ")
    elif filename.endswith("CMakeLists.txt"):
        replace_copyright_detail(filename, contributors, "# ")

def main(args):
    for filename in args:
        try:
            commits = git_blame(filename)
            contributors = extract_contributors(commits)
            replace_copyright(filename, contributors)
        except Exception as e:
            print("Error '{}' for file '{}'".format(e, filename))

if __name__ == "__main__":
    main(sys.argv[1:])
