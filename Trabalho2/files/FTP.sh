#!bin/sh

if [ -z $1 ]
then
    exec="ftpdownload"
elif [ $1 = "debug" ]
then
    exec="ftpdownload_debug"
fi

function anonymous_mode {
    echo '+----------------------+'
    echo '|Anonymous mode example|'
    echo '+----------------------+'

    echo -n "Default example? (Y/n)"
    read option

    if [[ -z $option || $option != "n" ]]
    then
        path="pub/CPAN/RECENT-1M.json"
    else
        echo -n "Path to file: "
        read path
    fi


    echo "---> Running ${exec}"
    ./$exec ftp://ftp.up.pt/$path
}

function user_mode {
    echo '+-----------------+'
    echo '|User mode example|'
    echo '+-----------------+'

    echo -n "Default example? (Y/n)"
    read option
    if [[ -z $option || $option != "n" ]]
    then
        echo -n "Password for 'up201304197': "
        read -s pwd
        echo
        url="ftp://[up201304197:${pwd}@]tom.fe.up.pt/Documents/RCOM_ftp_test1.txt"
    else
        echo -n "User: "
        read username
        echo -n "Password: "
        read -s password
        echo
        echo -n "Path to file: "
        read path

        credentials="${username}:${password}"
        url="ftp://[${credentials}@]tom.fe.up.pt/${path}"
    fi

    echo "---> Running ${exec}"
    ./$exec $url
}

echo "Mode to run"
echo "  1. Anonymous"
echo "  2. User"
echo -n "> "
read option
while [ -z $option ]
do
    read option
done

if [ $option = "1" ]
then
    anonymous_mode
elif [ $option = "2" ]
then
    user_mode
else
    echo "Invalid option"
fi
