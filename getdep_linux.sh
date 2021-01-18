case "$(uname -s)" in

Linux)
rm -rf dependencies
apt-get install unzip
wget https://raw.githubusercontent.com/JiayinCao/SORT/dependencies/OSL_Integration_Last/linux/dependencies.zip
unzip dependencies.zip
rm dependencies.zip
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
