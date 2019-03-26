case "$(uname -s)" in

Darwin)
rm -rf dependencies
curl -o dependencies.zip http://45.63.123.194/sort_dependencies/mac/dependencies.zip
unzip dependencies.zip
rm -rf __MACOSX
rm dependencies.zip
cd ..
;;

CYGWIN*|MINGW32*|MSYS*)
echo 'MS Windows'
;;

# Add here more strings to compare
# See correspondence table at the bottom of this answer

*)
echo 'other OS'
;;
esac
