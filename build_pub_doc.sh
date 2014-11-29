#!/bin/bash
#
# Build documentation for zmqpp and publish it.
# It will build documentation the current branch, but not for
# pull request.
# 
# The documentation repository layout looks like this:
#    develop/html
#    master/html
#  

if [ $TRAVIS_PULL_REQUEST != "false" ]; then
    echo "Skipping documentation build because this is a pull request"
    exit 0
fi

DEPLOY_URL="https://${GH_TOKEN}@github.com/xaqq/zmqpp-doc.git"

git clone $DEPLOY_URL -b gh-pages
doxygen > docs/doxygen-log || { echo "Failed to build doc"; exit 1; }

# Remove doc for current branch
rm -rf ${TRAVIS_BRANCH}

# Copy new documentation
mkdir -p zmqpp-doc/${TRAVIS_BRANCH}
cp -R docs/html zmqpp-doc/${TRAVIS_BRANCH}
# Copy doxygen log file.
cp docs/doxygen-log zmqpp-doc/${TRAVIS_BRANCH}

cd zmqpp-doc
git config user.email "travis-ci@automated-commit.org"
git config user.name "TravisCI"
git add html
git commit -am "Updating documentation for branch ${TRAVIS_BRANCH}"
git push origin gh-pages

exit 0
