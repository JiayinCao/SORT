case "$(uname -s)" in

Linux)
rm -rf dependencies
apt-get install unzip
wget http://45.63.123.194/sort_dependencies/linux/dependencies_18_04.zip
unzip dependencies_18_04.zip
rm dependencies_18_04.zip
cd ..
dir
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
