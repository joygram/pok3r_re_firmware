#include "pok3r_rgb.h"
#include "zlog.h"

bool Pok3rRGB::findPok3rRGB(){
    // Try firmware vid and pid
    if(findUSBVidPid(HOLTEK_VID, POK3R_RGB_PID))
        return true;
    return false;
}

ZString Pok3rRGB::getVersion(){
    ZBinary bin(PKT_LEN);
    if(!sendCmd(CMD_READ, 0x20, 0, nullptr, 0))
        return "ERROR";
    if(!recvDat(bin.raw()))
        return "ERROR";
    ZString ver;
    ver.parseUTF16((zu16 *)(bin.raw() + 8), 60);
    return ver;
}

ZBinary Pok3rRGB::dumpFlash(){
    ZBinary dump;
    for(zu16 i = 0; i < 0x10000 - 60; i += 60){
        ZBinary bin(PKT_LEN);
        if(!sendCmd(CMD_READ, 0xff, i, nullptr, 0))
            return dump;
        if(!recvDat(bin.raw()))
            return dump;

//        RLOG(bin.dumpBytes(4, 8));
        dump.write(bin.raw() + 4, 60);
    }
    return dump;
}

bool Pok3rRGB::sendCmd(zu8 cmd, zu8 a1, zu16 a2, const zbyte *data, zu8 len){
    if(len > 52)
        return false;

    ZBinary packet;
    packet.fill(0, PKT_LEN);
    packet.writeu8(cmd);    // command
    packet.writeu8(a1);     // argument
    packet.writeleu16(a2);  // patch argument

    if(data){
        packet.write(data, len);  // data
    }

    // debug
//    RLOG(packet.dumpBytes(4, 8));

    // Send command (interrupt write)
    zu16 olen = interrupt_send(SEND_EP, packet.raw(), PKT_LEN);

    if(olen != PKT_LEN){
        ELOG("Failed to send: length " << olen);
        return false;
    }

    return true;
}

bool Pok3rRGB::recvDat(zbyte *data){
    if(!data)
        return false;

    // Recv data (interrupt read)
    zu16 olen = interrupt_recv(RECV_EP, data, PKT_LEN);

    if(olen != PKT_LEN){
        ELOG("Failed to recv: length " << olen);
        return false;
    }

    // debug
//    RLOG(ZBinary(data, PKT_LEN).dumpBytes(4, 8));

    return true;
}