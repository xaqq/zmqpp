#!/bin/bash

DEPLOY_URL="https://${GH_TOKEN}@github.com/xaqq/zmqpp-doc.git"

git clone $DEPLOY_URL -b gh-pages
doxygen || { echo "Failed to build doc"; exit 1; }

#purge current doc
rm -rf zmqpp-doc/html

#cp new doc
cp -R docs/html

cd zmqpp-doc
git commit -am "Updating documentation"
git push

exit 0
