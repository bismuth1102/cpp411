
#include <iostream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <jsoncpp/json/json.h>

#include "twitcurl/twitterClient/include/twitcurl.h"

#include "cc.h"

using namespace std;
using namespace boost::asio;

io_service service;

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class write_t_db : 
public boost::enable_shared_from_this<write_t_db>
                  , boost::noncopyable {
    typedef write_t_db self_type;
    write_t_db(const string & message) 
      : sock_(service), started_(true), message_(message) {}
    void start(ip::tcp::endpoint ep) {
        sock_.async_connect(ep, MEM_FN1(on_connect,_1));
    }
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<write_t_db> ptr;

    static ptr start(ip::tcp::endpoint ep, const string & message) {
        ptr new_(new write_t_db(message));
        new_->start(ep);
        return new_;
    }
    void stop() {
        if ( !started_) return;
        started_ = false;
        sock_.close();
    }
    bool started() { return started_; }
private:
    void on_connect(const error_code & err) {
        if ( !err)      do_write(message_ + "\n");
        else            stop();
    }
    
    void do_write(const string & msg) {
        if ( !started() ) return;
        std::copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some( buffer(write_buffer_, msg.size()), 
                                MEM_FN2(on_write,_1,_2));
    }

    void on_write(const error_code & err, size_t bytes) {
        stop();
    }
    

private:
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    string message_;
};

int main(int argc, char* argv[]) {
    // connect several clients
    ip::tcp::endpoint ep( ip::address::from_string("127.0.0.1"), 8001);

    //use twitcurl here

    string key = "planet";
    string count = "10";
    string jsonStr = searchTwit("vancleecheng", "951102ljc", key, count);

    cout << jsonStr << endl;

    string results = "w";
    results = results += key + "`";

    Json::Reader reader;
    Json::Value root;

    //从字符串中读取数据
    if(reader.parse(jsonStr, root))
    {
        int statuses_size = root["statuses"].size();
        for(int i=0; i<statuses_size; i++){
            Json::Value status = root["statuses"][i];
            string time = status["created_at"].asString();
            vector<string> vStr;
            boost::split( vStr, time, boost::is_any_of( " " ), boost::token_compress_on);
            string month = parseMonth(vStr[1]);
            time = vStr[5]+"/"+month+"/"+vStr[2];
            results += time+";";
        }

    }

    cout << results << endl;
    write_t_db::start(ep, results);
    service.run();
}


string parseMonth(string month){
    if(month=="Jan")
        return "1";
    if(month=="Feb")
        return "w";
    if(month=="Mar")
        return "3";
    if(month=="Apr")
        return "4";
    if(month=="May")
        return "5";
    if(month=="Jun")
        return "6";
    if(month=="Jul")
        return "7";
    if(month=="Aut")
        return "8";
    if(month=="Sep")
        return "9";
    if(month=="Oct")
        return "10";
    if(month=="Nov")
        return "11";
    if(month=="Dec")
        return "12";
}


string searchTwit(string userName, string passWord, string searchStr, string count) {
    twitCurl twitterObj;
    std::string tmpStr, tmpStr2;
    std::string replyMsg;
    char tmpBuf[1024];

    /* Set twitter username and password */
    twitterObj.setTwitterUsername( userName );
    twitterObj.setTwitterPassword( passWord );

    /* Set proxy server usename, password, IP and port (if present) */
    memset( tmpBuf, 0, 1024 );
    

    /* OAuth flow begins */
    /* Step 0: Set OAuth related params. These are got by registering your app at twitter.com */
    twitterObj.getOAuth().setConsumerKey( std::string( "UpuV9gukS1xU4RsA9l79tjW9N" ) );
    twitterObj.getOAuth().setConsumerSecret( std::string( "Px5zwdnuSfzo6kpZON4SWqGO5vUkB81kWERKzXExG59wmEEw9p" ) );

    /* Step 1: Check if we alredy have OAuth access token from a previous run */
    std::string myOAuthAccessTokenKey("");
    std::string myOAuthAccessTokenSecret("");
    std::ifstream oAuthTokenKeyIn;
    std::ifstream oAuthTokenSecretIn;

    oAuthTokenKeyIn.open( "twitcurl/twitterClient/twitterClient_token_key.txt" );
    oAuthTokenSecretIn.open( "twitcurl/twitterClient/twitterClient_token_secret.txt" );

    memset( tmpBuf, 0, 1024 );
    oAuthTokenKeyIn >> tmpBuf;
    myOAuthAccessTokenKey = tmpBuf;

    memset( tmpBuf, 0, 1024 );
    oAuthTokenSecretIn >> tmpBuf;
    myOAuthAccessTokenSecret = tmpBuf;

    oAuthTokenKeyIn.close();
    oAuthTokenSecretIn.close();

    if( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
    {
        /* If we already have these keys, then no need to go through auth again */
        // printf( "\nUsing:\nKey: %s\nSecret: %s\n\n", myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str() );

        twitterObj.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
        twitterObj.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );
    }
    else
    {
        /* Step 2: Get request token key and secret */
        std::string authUrl;
        twitterObj.oAuthRequestToken( authUrl );

        /* Step 3: Get PIN  */
        memset( tmpBuf, 0, 1024 );
        printf( "\nDo you want to visit twitter.com for PIN (0 for no; 1 for yes): " );
        fgets( tmpBuf, sizeof( tmpBuf ), stdin );
        tmpStr = tmpBuf;
        if( std::string::npos != tmpStr.find( "1" ) )
        {
            /* Ask user to visit twitter.com auth page and get PIN */
            memset( tmpBuf, 0, 1024 );
            printf( "\nPlease visit this link in web browser and authorize this application:\n%s", authUrl.c_str() );
            printf( "\nEnter the PIN provided by twitter: " );
            fgets( tmpBuf, sizeof( tmpBuf ), stdin );
            tmpStr = tmpBuf;
            twitterObj.getOAuth().setOAuthPin( tmpStr );
        }
        else
        {
            /* Else, pass auth url to twitCurl and get it via twitCurl PIN handling */
            twitterObj.oAuthHandlePIN( authUrl );
        }

        /* Step 4: Exchange request token with access token */
        twitterObj.oAuthAccessToken();

        /* Step 5: Now, save this access token key and secret for future use without PIN */
        twitterObj.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
        twitterObj.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

        /* Step 6: Save these keys in a file or wherever */
        std::ofstream oAuthTokenKeyOut;
        std::ofstream oAuthTokenSecretOut;

        oAuthTokenKeyOut.open( "twitcurl/twitterClient/twitterClient_token_key.txt" );
        oAuthTokenSecretOut.open( "twitcurl/twitterClient/twitterClient_token_secret.txt" );

        oAuthTokenKeyOut.clear();
        oAuthTokenSecretOut.clear();

        oAuthTokenKeyOut << myOAuthAccessTokenKey.c_str();
        oAuthTokenSecretOut << myOAuthAccessTokenSecret.c_str();

        oAuthTokenKeyOut.close();
        oAuthTokenSecretOut.close();
    }
    /* OAuth flow ends */

    /* Account credentials verification */
    if( twitterObj.accountVerifyCredGet() )
    {
        twitterObj.getLastWebResponse( replyMsg );
        // printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet web response:\n%s\n", replyMsg.c_str() );
    }
    else
    {
        twitterObj.getLastCurlError( replyMsg );
        printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet error:\n%s\n", replyMsg.c_str() );
    }

    /* Get followers' ids */
    std::string nextCursor("");
    std::string searchUser("nextbigwhat");
    do
    {
        if( twitterObj.followersIdsGet( nextCursor, searchUser ) )
        {
            twitterObj.getLastWebResponse( replyMsg );
            // printf( "\ntwitterClient:: twitCurl::followersIdsGet for user [%s] web response:\n%s\n",
                    // searchUser.c_str(), replyMsg.c_str() );

            // JSON: "next_cursor":1422208797779779359,
            nextCursor = "";
            size_t nNextCursorStart = replyMsg.find("next_cursor");
            if( std::string::npos == nNextCursorStart )
            {
                nNextCursorStart += strlen("next_cursor:\"");
                size_t nNextCursorEnd = replyMsg.substr(nNextCursorStart).find(",");
                if( std::string::npos != nNextCursorEnd )
                {
                    nextCursor = replyMsg.substr(nNextCursorStart, (nNextCursorEnd - nNextCursorStart));
                    // printf("\nNEXT CURSOR: %s\n\n\n\n\n", nextCursor.c_str());
                }
            }
        }
        else {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::followersIdsGet error:\n%s\n", replyMsg.c_str() );
            break;
        }
    } while( !nextCursor.empty() && nextCursor.compare("0") );

    /* Get block list */
    nextCursor = "";
    if( twitterObj.blockListGet( nextCursor, false, false ) )
    {
        twitterObj.getLastWebResponse( replyMsg );
        // printf( "\ntwitterClient:: twitCurl::blockListGet web response:\n%s\n", replyMsg.c_str() );
    }
    else
    {
        twitterObj.getLastCurlError( replyMsg );
        printf( "\ntwitterClient:: twitCurl::blockListGet error:\n%s\n", replyMsg.c_str() );
    }

    /* Get blocked ids */
    nextCursor = "";
    if( twitterObj.blockIdsGet( nextCursor, true ) )
    {
        twitterObj.getLastWebResponse( replyMsg );
        // printf( "\ntwitterClient:: twitCurl::blockIdsGet web response:\n%s\n", replyMsg.c_str() );
    }
    else
    {
        twitterObj.getLastCurlError( replyMsg );
        printf( "\ntwitterClient:: twitCurl::blockIdsGet error:\n%s\n", replyMsg.c_str() );
    }

    /* Search a string */
    // printf( "\nEnter string to search: " );
    // memset( tmpBuf, 0, 1024 );
    // fgets( tmpBuf, sizeof( tmpBuf ), stdin );
    // tmpStr = tmpBuf;
    // tmpStr = tmpStr.substr(0, tmpStr.length() - 1);
    // printf( "\nLimit search results to: " );
    // memset( tmpBuf, 0, 1024 );
    // fgets( tmpBuf, sizeof( tmpBuf ), stdin );
    // tmpStr2 = tmpBuf;
    // tmpStr2 = tmpStr2.substr(0, tmpStr2.length() - 1);

    replyMsg = "";
    if( twitterObj.search( searchStr , count ) )
    {
        twitterObj.getLastWebResponse( replyMsg );
        cout << "right" << endl;
        return replyMsg.c_str();
        // printf( "\ntwitterClient:: twitCurl::search web response:\n%s\n", replyMsg.c_str() );
    }
    else
    {   
        twitterObj.getLastCurlError( replyMsg );
        cout << "wrong" << endl;
        return replyMsg.c_str();
        // printf( "\ntwitterClient:: twitCurl::search error:\n%s\n", replyMsg.c_str() );

    }

    #ifdef _TWITCURL_TEST_
        /* Get user timeline */
        replyMsg = "";
        printf( "\nGetting user timeline\n" );
        if( twitterObj.timelineUserGet( true, true, 0 ) )
        {
            twitterObj.getLastWebResponse( replyMsg );
            printf( "\ntwitterClient:: twitCurl::timelineUserGet web response:\n%s\n", replyMsg.c_str() );
        }
        else
        {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::timelineUserGet error:\n%s\n", replyMsg.c_str() );
        }

        /* Destroy a status message */
        memset( tmpBuf, 0, 1024 );
        printf( "\nEnter status message id to delete: " );
        fgets( tmpBuf, sizeof( tmpBuf ), stdin );
        tmpStr = tmpBuf;
        replyMsg = "";
        if( twitterObj.statusDestroyById( tmpStr ) )
        {
            twitterObj.getLastWebResponse( replyMsg );
            printf( "\ntwitterClient:: twitCurl::statusDestroyById web response:\n%s\n", replyMsg.c_str() );
        }
        else
        {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::statusDestroyById error:\n%s\n", replyMsg.c_str() );
        }

        /* Get public timeline */
        replyMsg = "";
        printf( "\nGetting public timeline\n" );
        if( twitterObj.timelinePublicGet() )
        {
            twitterObj.getLastWebResponse( replyMsg );
            printf( "\ntwitterClient:: twitCurl::timelinePublicGet web response:\n%s\n", replyMsg.c_str() );
        }
        else
        {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::timelinePublicGet error:\n%s\n", replyMsg.c_str() );
        }

        /* Get friend ids */
        replyMsg = "";
        printf( "\nGetting friend ids\n" );
        tmpStr = "techcrunch";
        if( twitterObj.friendsIdsGet( tmpStr, false ) )
        {
            twitterObj.getLastWebResponse( replyMsg );
            printf( "\ntwitterClient:: twitCurl::friendsIdsGet web response:\n%s\n", replyMsg.c_str() );
        }
        else
        {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::friendsIdsGet error:\n%s\n", replyMsg.c_str() );
        }

        /* Get trends */
        if( twitterObj.trendsDailyGet() )
        {
            twitterObj.getLastWebResponse( replyMsg );
            printf( "\ntwitterClient:: twitCurl::trendsDailyGet web response:\n%s\n", replyMsg.c_str() );
        }
        else
        {
            twitterObj.getLastCurlError( replyMsg );
            printf( "\ntwitterClient:: twitCurl::trendsDailyGet error:\n%s\n", replyMsg.c_str() );
        }
    #endif // _TWITCURL_TEST_

        return 0;
}

