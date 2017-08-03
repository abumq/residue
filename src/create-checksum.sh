find . -name "*.cc" > .full
find . -name "*.h" >> .full
shasum .full > full.sha1
