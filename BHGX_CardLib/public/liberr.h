#pragma once

#ifndef _CARD_LIB_ERR_H_
#define _CARD_LIB_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

enum ErrType
{
	CardProcSuccess = 0,
	CardInitErr,
	CardXmlErr,
	CardReadErr,
	CardScanErr,
	CardWriteErr,
	CardCoverPrintErr,
	CardDLLLoadErr,
	CardCreateErr,
	CardFormatErr,
	CardCreateDateErr,
	FeedCardError,
	CardCheckError,
	CardRegError,
	EncryFileError,
	DescryFileError,
	CardIsNotEmpty,
	CardNotOpen,
	CardNoAuthority,
	CardAuthExpired,
	CardCreateLicenseFailed,
	CardNoSupport,
	CardMallocFailed,
	CardForbidden,
	CardWarnning,
	CardMedicalFailed,
	CardNotMedicalCard,
	ConnectWebServerFailed,
	CardSQLError,
	CardDBFileNotFound,
	CardDBConnectError,
	CardDBInsertError,
	CardInputParamError,
};

static char *errString[] =
{
	"卡操作成功",				// 0
	"卡初始化失败",			// -1
	"XML文件格式有误",		// -2
	"卡读取失败",		// -3
	"寻卡失败",		// -4
	"卡写入失败",		//-5
	"打印卡面失败",	//-6
	"卡打印动态链接库未接入",	//-7
	"制卡失败",			//-8
	"卡格式化失败",				//-9
	"生成卡数据失败",			//-10
	"进卡失败",					// -11
	"卡校验失败",				// -12	
	"卡注册失败",				//-13
	"加密文件失败",				//-14
	"解密文件失败,没有此文件，或者不是加密文件",				//-15
	"卡不是空卡",
	"设备没有被打开",
	"没有被授权操作，请联系供应商",
	"授权已经过期，请联系供应商",
	"生成license文件错误，请检查时间格式",
	"此卡暂没有此功能",
	"分配内存错误",
	"该卡已注销,无法使用",
	"该卡已挂失，暂时无法使用",
	"农合号为空",
	"非农合卡，请重新换卡",
	"连接webserver失败",
	"查询数据库失败",
	"数据库文件不存在，请确认数据库*.db存在",
	"数据库连接失败，请确认数据库网络是否正常",
	"数据库插入数据失败",
	"函数入参为空，请检查入参",
};

char *_err(int errcode);

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_ERR_H_