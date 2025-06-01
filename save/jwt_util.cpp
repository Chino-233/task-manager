#include "jwt_util.h"
#include <chrono>

std::string JWTUtil::generateToken(const std::string &userId, const std::string &secret) {
    // 获取当前时间点而非时间段
    auto now = std::chrono::system_clock::now();
    
    return jwt::create()
        .set_issuer("task_manager")
        .set_issued_at(now)  // 使用时间点
        .set_expires_at(now + std::chrono::hours(24)) // 24小时后过期
        .set_payload_claim("user_id", jwt::basic_claim(userId)) // 正确语法
        .sign(jwt::algorithm::hs256{secret});
}

jwt::decoded_jwt<jwt::traits::kazuho_picojson> JWTUtil::verifyToken(
    const std::string &token, 
    const std::string &secret
) {
    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{secret})
        .with_issuer("task_manager");
    
    auto decoded = jwt::decode<jwt::traits::kazuho_picojson>(token);
    verifier.verify(decoded);
    return decoded;  // 返回类型应与函数声明一致
}