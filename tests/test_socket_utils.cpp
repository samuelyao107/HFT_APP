#include <gtest/gtest.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "common/socket_utils.hpp" 

TEST(NetworkUtilsTest, SetNonBlocking_ApplyTheRightFLag) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    ASSERT_NE(fd, -1) << "Failed to create the socket!";

    bool result = Common::setNonBlocking(fd);


    EXPECT_TRUE(result) << "The function returns false!";

  
    int current_flags = fcntl(fd, F_GETFL, 0);
    ASSERT_NE(current_flags, -1) << "Impossible to read the flags";
    
    EXPECT_TRUE(current_flags & O_NONBLOCK) << "Le drapeau O_NONBLOCK n'est pas posé sur le socket !";

    close(fd);
}

TEST(NetworkUtilsTest, SetNoDelay_DisableNaggleAlgorithm) {
   
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(fd, -1) << "Failed to create the socket";

    bool result = Common::setNoDelay(fd);
    EXPECT_TRUE(result) << "setNoDelay returned false";

    int optval = 0; 
    socklen_t optlen = sizeof(optval);


    int ret = getsockopt(fd,IPPROTO_TCP, TCP_NODELAY,&optval, &optlen);

    ASSERT_NE(ret,-1);

    EXPECT_EQ(1,optval)<<"Naggle couldnt be disabled";

    close(fd);
}

TEST(NetworkUtilsTest, SetTTLTest){

     int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    ASSERT_NE(fd, -1) << "Failed to create the socket!";

    bool result = Common::setTTL(fd,1);

    EXPECT_TRUE(result)<<"setTTL failed!";

    int optval =0;
    socklen_t optlen = sizeof(optval);

    int ret = getsockopt(fd, IPPROTO_IP, IP_TTL, &optval,&optlen);

    ASSERT_NE(ret,-1);

    EXPECT_EQ(1,optval)<<"TTL couldnt be set";

}

TEST(NetworkUtilsTest, SetMcastTTLTest){

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    ASSERT_NE(fd, -1) << "Failed to create the socket!";

    bool result = Common::setMcastTTL(fd,1);

    EXPECT_TRUE(result)<<"setMcastTTL failed!";

    int optval =0;
    socklen_t optlen = sizeof(optval);

    int ret = getsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &optval,&optlen);

    ASSERT_NE(ret,-1);

    EXPECT_EQ(1,optval)<<"McastTTL couldnt be set";

}

TEST(NetworkUtilsTest, SetSOTimestampTest){

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    ASSERT_NE(fd, -1) << "Failed to create the socket!";

    bool result = Common::setSOTimestamp(fd);

    EXPECT_TRUE(result)<<"setSOTimestamp failed!";

    int optval =0;
    socklen_t optlen = sizeof(optval);

    int ret = getsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &optval,&optlen);

    ASSERT_NE(ret,-1);

    EXPECT_EQ(1,optval)<<"SOT Timestamp couldnt be set";

}