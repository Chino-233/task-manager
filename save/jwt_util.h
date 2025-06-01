#pragma once
#include <string>
#include <jwt-cpp/jwt.h>

class JWTUtil {
public:
    // 添加模板参数
    static std::string generateToken(const std::string &userId, const std::string &secret);
    
    // 修改返回类型或添加模板参数
    static jwt::decoded_jwt<jwt::traits::kazuho_picojson> verifyToken(
        const std::string &token, 
        const std::string &secret
    );
};