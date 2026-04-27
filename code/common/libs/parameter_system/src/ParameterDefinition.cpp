#include "parameter_system/ParameterDefinition.h"

namespace parameter_system {

ReadWriteResult ParameterDefinition::setValueRaw(std::span<uint8_t> buff) {
    if (!valueIsWritable()) return ReadWriteResult::not_allowed;
    if (buff.size_bytes() < pointed_data_size_) return ReadWriteResult::buffer_size_mismatch;

    std::memcpy(data_ptr_, buff.data(), pointed_data_size_);

    if (on_change_callback_ != nullptr) {
        on_change_callback_();
    }

    return ReadWriteResult::ok;
}

ReadWriteResult ParameterDefinition::getValueRaw(std::span<uint8_t> target_buff, size_t* written_bytes_out) {
    if (target_buff.size_bytes() < pointed_data_size_) return ReadWriteResult::buffer_size_mismatch;

    std::memcpy(target_buff.data(), data_ptr_, pointed_data_size_);
    if (written_bytes_out != nullptr) *written_bytes_out = pointed_data_size_;

    return ReadWriteResult::ok;
}

}  // namespace parameter_system
