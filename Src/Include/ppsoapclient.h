// PPSOAPCLIENT.H
// Copyright (c) A.Sobolev 2012, 2013, 2014, 2015, 2016, 2017
//
#include <slib.h>

BOOL Implement_SoapModule_DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved, const char * pProductName);

class PPSoapResultPtrBase {
public:
	static SCollection ResultPtrList;

	virtual void Destroy() = 0;
	virtual const void * GetPtr() const = 0;
	int operator == (const void * ptr) const
	{
		return (ptr == GetPtr());
	}
};

template <class T> class PPSoapResultPtr : public PPSoapResultPtrBase {
public:
	PPSoapResultPtr(T * p)
	{
		Ptr = p;
	}
	virtual void Destroy()
	{
		if(Ptr) {
			delete Ptr;
			Ptr = 0;
		}
	}
	virtual const void * GetPtr() const
	{
		return Ptr;
	}
	T * Ptr;
};

int FASTCALL PPSoapDestroyResultPtr(void * p);
void ** FASTCALL PPSoapCreateArray(uint count, int & rArrayCount);

template <class T> static void PPSoapRegisterResultPtr(T * ptr)
{
	if(ptr) {
		PPSoapResultPtr <T> * p_item = new PPSoapResultPtr <T> (ptr);
		if(p_item)
			PPSoapResultPtrBase::ResultPtrList.insert(p_item);
	}
}


class PPSoapClientSession {
public:
	PPSoapClientSession();
	void   Setup(const char * pUrl);
	void   Setup(const char * pUrl, const char * pUser, const char * pPassword);
	const  char * GetUrl() const;
	void   FASTCALL SetMsg(const char * pUtf8Text);
	const  char * GetMsg() const;
	const  char * GetUser() const;
	const  char * GetPassword() const;
private:
	int    ErrCode;
	char   ErrMsg[1024];
	char   Url[256];
	SString User;
	SString Password;
};

class InParamString : public STempBuffer {
public:
	InParamString(const char * pStr);
};

char * FASTCALL GetDynamicParamString(const char * pSrc, TSCollection <InParamString> & rPool);
char * FASTCALL GetDynamicParamString(long ival, TSCollection <InParamString> & rPool);
char * FASTCALL GetDynamicParamString(int ival, TSCollection <InParamString> & rPool);
char * FASTCALL GetDynamicParamString_(double rval, long fmt, TSCollection <InParamString> & rPool);
char * FASTCALL GetDynamicParamString(LDATE dval, long fmt, TSCollection <InParamString> & rPool);
char * FASTCALL GetDynamicParamString(LDATETIME dtval, long dfmt, long tfmt, TSCollection <InParamString> & rPool);

class UhttStatus {
public:
	UhttStatus();
	int    Code;    // ��� ���������� ���������� ��������
	int    Index;   // ��� ������ - ������ � �������� ������ (���������� � ���� ���������), � �������� ��������� ������
	int    Id;      // ��� ������ - ������������� ������� � �������� ������ (���������� � ���� ���������), � �������� ��������� ������
	SString Msg;    // ����� ��������� //
};

class UhttDate {
public:
	UhttDate & Clear();
	UhttDate & FASTCALL operator = (const char * pStr);
	UhttDate & FASTCALL operator = (LDATE dt);
	operator LDATE () const;

	SString Date;
};
//
// Descr: ����/�����, ������������ ����� ������� � ������� ISO-8601 (yyyy-mm-ddThh:mm:ss)
//
class UhttTimestamp {
public:
	UhttTimestamp & Clear();
	UhttTimestamp & FASTCALL operator = (LDATE dt);
	UhttTimestamp & FASTCALL operator = (LTIME tm);
	UhttTimestamp & FASTCALL operator = (const LDATETIME & rDtm);
	operator LDATE () const;
	operator LTIME () const;
	operator LDATETIME () const;

	SString T;
};

class UhttDateTime {
public:
	UhttDateTime & Clear();
	UhttDateTime & FASTCALL operator = (LDATE dt);
	UhttDateTime & FASTCALL operator = (LTIME tm);
	UhttDateTime & FASTCALL operator = (const LDATETIME & rDtm);
	operator LDATE () const;
	operator LTIME () const;
	operator LDATETIME () const;
	SString Date;
	SString Time;
};

class UhttDatePeriod {
public:
	UhttDatePeriod & Clear();
	UhttDatePeriod & FASTCALL operator = (const DateRange & rRng);
	SString Low;
	SString Upp;
};

class UhttTagItem {
public:
	UhttTagItem();
	UhttTagItem(const char * pSymb, const SString & rText);
	void FASTCALL SetSymb(const char * pText);
	void FASTCALL SetValue(const char * pText);
	SString Symb;
	SString Value;
};

class UhttBrandPacket {
public:
	UhttBrandPacket();

	int32  ID;
	char   Name[128];
	int32  OwnerID;
	int32  Flags;
};

class UhttStyloDevicePacket {
public:
	UhttStyloDevicePacket();
	void FASTCALL SetName(const char * pText);
	void FASTCALL SetSymb(const char * pText);
	int    ID;
	SString Name;
	SString Symb;
	int    ParentID;
	int    Flags;
	int    DeviceVer;
	UhttTimestamp RegisterTime;
};

class UhttGoodsPacket {
public:
	UhttGoodsPacket();
	UhttGoodsPacket & FASTCALL operator = (const UhttGoodsPacket & rS);
	UhttGoodsPacket & FASTCALL Copy(const UhttGoodsPacket & rS);
	//
	// Descr: ������������� ������������ ������ (� �������������� � �������������� �� ������ ��������);
	// Note: �������� ������ �� ������� Papyrus
	//
	void    FASTCALL SetName(const char * pName);
	//
	// Descr: ������������� ����� ���������� ������ (� �������������� � �������������� �� ������ ��������);
	// Note: �������� ������ �� ������� Papyrus
	//
	void    FASTCALL SetExt(int extFldId, const char * pText);

	struct Barcode {
		int32  GoodsID;
		char   Code[32];
		double Qtty;
	};
	int32  ID;
	int32  BrandID;
	int32  ManufID; // @v8.3.5
	char   Name[256];
	char   SingleBarcode[32];
	double Package;

	SString ExtA; // STORAGE
	SString ExtB; // STANDARD
	SString ExtC; // INGRED
	SString ExtD; // ENERGY
	SString ExtE; // USAGE
	TSArray <UhttGoodsPacket::Barcode> BarcodeList;
	TSCollection <UhttTagItem> TagList;
};
//
// Descr: ������������� �������� ������������� ���������� �������������� ������� � ���������������
//   ���������������� ������� � Universe-HTT �� ����.
//
struct UhttCodeRefItem {
	UhttCodeRefItem();
	UhttCodeRefItem & FASTCALL Set(int privateID, const char * pCode);

	int32  PrivateID;
	int32  UhttID;
	uint   InnerPos; // ���������� ����. ������������ ��� ������ WSDL-������ ��� ������������� �������.
	char   Code[32];
};

class UhttDocumentPacket {
public:
	UhttDocumentPacket();
	int    SetFile(const char * pFileName);

	int    UhttObjID;
	int64  Size;
	SString ObjTypeSymb;
	SString Name;
	SString Encoding;
	SString ContentType;
	SString ContentMime;
};

class UhttSpecSeriesPacket {
public:
	UhttSpecSeriesPacket();
	void FASTCALL SetSerial(const char * pText);
	void FASTCALL SetGoodsName(const char * pText);
	void FASTCALL SetManufName(const char * pText);
	void FASTCALL SetInfoIdent(const char * pText);
	void FASTCALL SetAllowNumber(const char * pText);
	void FASTCALL SetLetterType(const char * pText);

	int     ID;
	int     GoodsID;
	int     ManufID;
	int     ManufCountryID;
	int     LabID;
	SString Serial;
	SString Barcode;
	SString GoodsName;
	SString ManufName;
	SString InfoDate;
	int     InfoKind;
	SString InfoIdent;
	SString AllowDate;
	SString AllowNumber;
	SString LetterType;
	int     Flags;
};

class UhttLocationPacket {
public:
	UhttLocationPacket();
	void FASTCALL SetName(const char * pName);
	void FASTCALL SetCode(const char * pCode);
	void FASTCALL SetPostalCode(const char * pPostalCode);
	void FASTCALL SetAddress(const char * pAddress);
	void FASTCALL SetPhone(const char * pPhone);
	void FASTCALL SetEMail(const char * pEMail);
	void FASTCALL SetContact(const char * pContact);
	SString & FASTCALL GetCode(SString & rBuf) const;

	int    ID;
	int    ParentID;
	int    Type;
	int    CityID;
	int    OwnerPersonID;
	double Latitude;
	double Longitude;
	int    Flags;
	SString Name;
	SString Code;
	SString PostalCode;
	SString Address;
	SString Phone;
	SString EMail;
	SString Contact;
};

class UhttSCardPacket {
public:
	UhttSCardPacket();
	void FASTCALL SetCode(const char * pCode);

	int    ID;
	int    SeriesID;
	int    OwnerID;
	SString Code;
	SString Hash;
	UhttDateTime IssueDate;
	UhttDateTime Expiry;
	double PDis;
	double Overdraft;
	double Debit;
	double Credit;
	double Rest;
	int    Flags;
};

class UhttCheckPacket {
public:
	UhttCheckPacket();

	struct CheckItem {
		int    ID;
		int    GoodsID;
		int    Qtty;
		double Price;
		double Discount;
	};
	int    PosNodeID;
	int    LocID;
	int    SCardID;
	double Amount;
	double Discount;
	int    Flags;
	UhttDateTime Dtm;
	TSArray <UhttCheckPacket::CheckItem> Items;
};

class UhttQuotFilter {
public:
	UhttQuotFilter();

	int    GoodsID;
	int    GroupID;
	int    BrandID;
	int    SellerID;
	int    LocationID;
	int    BuyerID;
	int    AndFlags;
	int    NotFlags;
};

class UhttQuotPacket {
public:
	UhttQuotPacket();

	int    GoodsID;
	int    SellerID;
	int    LocID;
	int    BuyerID;
	int    CurrID;
	double MinQtty;
	UhttDateTime ChangesDate;
	UhttDatePeriod ActualPeriod;
	double Value;
	int    Flags;
};

class UhttPersonPacket {
public:
	UhttPersonPacket();
	UhttPersonPacket & operator = (const UhttPersonPacket & rS);
	SString & GetUhttContragentCode(SString & rBuf) const;

	struct Kind {
		SString Code;
		SString Name;
	};
	struct Register {
		int    ID;
		int    TypeID;
		int    OrganID;
		UhttDate Dt;
		UhttDate Expiry;
		SString TypeName;
		SString Serial;
		SString Number;
	};
	struct AddressP : public UhttLocationPacket {
		AddressP();

		int    Kind; // 1 - �� �����, 2 - ����������� �����, 3 - ����� ��������
		SString CountryName;
		SString CityName;
	};
	int    ID;
	int    StatusID;
	int    CategoryID;
	SString Name;
	SString INN;
	StringSet PhoneList;
	StringSet EMailList;
	StringSet UrlList;
	TSCollection <Kind> KindList;
	TSCollection <Register> RegList;
	TSCollection <AddressP> AddrList;
};

class UhttBillPacket {
public:
	UhttBillPacket();

	struct BillItem {
		int     BillID;
		int     GoodsID;
		int     Flags;
		double  Quantity;
		double  Cost;
		double  Price;
		double  Discount;
		double  Amount;
		char    Serial[32];
	};
	int    ID;
	int    LocID;
	int    ArID;
	int    DlvrLocID;
	int    CurrID;
	int    AgentID;
	int    StatusID;
	int    Flags;
	S_GUID Uuid;
	UhttDateTime Dtm;
	SString Code;
	SString OpSymb;
	SString Memo;
	UhttPersonPacket Contractor;
	TSArray <UhttBillPacket::BillItem> Items;
};

class UhttBillFilter {
public:
	UhttBillFilter();
	int  FASTCALL SetDate(LDATE dt);
	void FASTCALL SetOpSymb(const char * pOpSymb);

	int     LocID;
	int     ArID;
	int     CurrID;
	int     AgentID;
	int     Count;
	int     Last;
	UhttDateTime Since; // @v8.3.2
	UhttDatePeriod Period;
	SString Dt;
	SString OpSymb;
};

class UhttGoodsRestListItem {
public:
	UhttGoodsRestListItem();

	int    GoodsID;
	int    LocID;
	SString Name;     // ������������ �����������
	SString LocCode;  // ��� �������
	SString LocAddr;  // ����� �������
	double Rest;
	double Price;
	UhttDateTime RestDtm;
	UhttDateTime PriceDtm;
};

class UhttDCFileVersionInfo {
public:
	UhttDCFileVersionInfo();

	int     ID;
	SString Key;
	SString Title;
	SString Name;
	SString Label;
	int     Revision;
	int64   Ts;
	long    Size;
	int     Flags;
	SString Memo;
};

class UhttWorkbookItemPacket {
public:
	UhttWorkbookItemPacket();
	UhttWorkbookItemPacket & FASTCALL operator = (const UhttWorkbookItemPacket & rS);
	UhttWorkbookItemPacket & FASTCALL Copy(const UhttWorkbookItemPacket & rS);
    void   FASTCALL SetName(const char * pText);
    void   FASTCALL SetSymb(const char * pText);
    void   FASTCALL SetVersion(const char * pText);
    void   FASTCALL SetDescr(const char * pText);

	int    ID;
	int    Type;
	int    ParentID;
	int    LinkID;
	int    CssID;
	int    Rank;
	int    Flags;
	int    KeywordCount;
	int    KeywordDilute;
	SString Name;
	SString Symb;
	UhttDateTime Dtm;
	UhttDateTime ModifDtm;        // @v9.3.8
	UhttDateTime ContentModifDtm; // @v9.3.7
	SString Version;
	SString Descr;
	TSCollection <UhttTagItem> TagList;
};
//
//
//
class UhttPrcPlaceDescription {
public:
	UhttPrcPlaceDescription();
	void FASTCALL SetRange(const char * pText);
	void FASTCALL SetDescr(const char * pText);

	int    GoodsID;    // �����, ��������������� � ���������� ����
	SString Range;
	SString Descr;
};

class UhttProcessorPacket {
public:
	UhttProcessorPacket();
	UhttProcessorPacket & FASTCALL operator = (const UhttProcessorPacket & rS);
	UhttProcessorPacket & FASTCALL Copy(const UhttProcessorPacket & rS);
	void FASTCALL SetName(const char * pName);
	void FASTCALL SetSymb(const char * pSymb);

	int    ID;
	SString Name;
	SString Symb;
	int    ParentID;
	int    Kind;
	int    Flags;
	int    LocID;
	int    LinkObjType;
	int    LinkObjID;
	int    CipPersonKindID;
	int    CipMax;
	TSCollection <UhttPrcPlaceDescription> Places;
};

class UhttTSessLine {
public:
	UhttTSessLine();

	int    TSessID;
	int    OprNo;
	int    GoodsID;
	int    LotID;
	int    UserID;
	int    Sign;
	UhttTimestamp Tm;
	int    Flags;
	double Qtty;
	double WtQtty;
	double Price;
	double Discount;
	UhttTimestamp Expiry;
	SString Serial;
};

class UhttCipPacket {
public:
	UhttCipPacket();
	void FASTCALL SetMemo(const char * pText);
	void FASTCALL SetPlaceCode(const char * pText);

	int    ID;     // ������������� ����������. � �������� ������ ������� �� ������ ���������������� ���� �����.
	int    Kind;   // ��� ����������.
	int    PrmrID; // ������������� ���������� �������, ��� �������� ����������� ����������� (��������, PPObjTSession)
	int    PersonID; // ->Person.ID
	int    Num;      // ����� ������ � ���������� ������ ������������ ��������� ���������� �������
	int    RegCount; // ���������� ����������, ���������� � ������ �����������. ��� ������� =1,
		// �� � ��������� ������� ����� ���� >1, ��������, ��� ��������� ��� ��������� �����������
	int    CiCount;  // ���������� ����������, ���������� � ������ ����������� �� �������� CheckedIn. ��� ������� =(0|1),
		// �� � ��������� ������� ����� ���� >1, ��������, ��� ��������� ��� ��������� �����������
	int    Flags;   // @flags
	UhttTimestamp RegTm;  // ����� ����������� //
	UhttTimestamp CiTm;   // ����� ������������� ����������� (CheckID)
	double Amount;   // �����, ���������� ��� ������� ������ ���� �������� �� ����������� (�������������)
	int    CCheckID; // �������� ���, ������� �������� ������������� �����������
	int    SCardID;  // �����, � ������� ������������� ������������������ ���������� //
	SString Memo;      // ����������
	SString PlaceCode; // ����� ����� (��� �����������, ��������������� � ���������� ������)
};

class UhttTSessionPacket {
public:
	UhttTSessionPacket();
	UhttTSessionPacket & FASTCALL operator = (const UhttTSessionPacket & rS);
	UhttTSessionPacket & FASTCALL Copy(const UhttTSessionPacket & rS);
	void FASTCALL SetMemo(const char * pText);
	void FASTCALL SetDetail(const char * pText);

	int    ID;
	int    Num; // ����� ������ (���������� �� ����������)
	int    PrcID;
	int    TechID;
	int    ParentID;
	int    Status;
	int    Flags;
	UhttTimestamp StTime;
	UhttTimestamp FinTime;
	SString Memo;
	SString Detail;
	TSCollection <UhttTSessLine> Lines;
	TSCollection <UhttCipPacket> Cips;
	TSCollection <UhttPrcPlaceDescription> Places;
	TSCollection <UhttTagItem> TagList;
};

class UhttSmsPacket {
public:
	SString From;
	SString To;
	SString Text;
};
//
// Descr: �������������� ��������� openssl
//
int SLAPI gSoapSslClientContextInit(struct soap * pSoap, const char * pKeyfile);
//
// Descr: �������������� ����� ������� gSoap
//
int SLAPI gSoapClientInit(struct soap * pSoap, long flags, const char * pKeyfile);

typedef int (*UHTT_DESTROYRESULT)(void *);

typedef SString * (*UHTTAUTH_PROC)(PPSoapClientSession & rSess, const char * pName, const char * pPassword);
typedef UhttPersonPacket * (*UHTTGETPERSONBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int id);
typedef TSCollection <UhttPersonPacket> * (*UHTTGETPERSONBYNAME_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pName);
typedef TSCollection <UhttBrandPacket> * (*UHTTGETBRANDBYNAME_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pName);
typedef int (*UHTTGETGOODSREFLIST_PROC)(PPSoapClientSession & rSess, const char * pToken, TSArray <UhttCodeRefItem> & rList);
typedef TSCollection <UhttGoodsPacket> * (*UHTTGETGOODSBYCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef TSCollection <UhttGoodsPacket> * (*UHTTGETGOODSBYNAME_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pName);
typedef UhttGoodsPacket * (*UHTTGETGOODSBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int id);
typedef SString *    (*UHTTGETGOODSARCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pGoodsCode, const char * pPersonINN);
typedef UhttStatus * (*UHTTCREATEGOODS_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttGoodsPacket & rPack);
typedef UhttStatus * (*UHTTSETIMAGEBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, UhttDocumentPacket & rPack);
typedef TSCollection <UhttSpecSeriesPacket> * (*UHTTGETSPECSERIESBYPERIOD_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pPeriod);
typedef UhttStatus * (*UHTTCREATESPECSERIES_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttSpecSeriesPacket & rPack);
typedef UhttSCardPacket * (*UHTTGETSCARDBYNUMBER_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pNumber);
typedef UhttStatus * (*UHTTCREATESCARDCHECK_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pLocSymb, const char * pSCardNumber, const UhttCheckPacket & rPack);
typedef UhttStatus * (*UHTTDEPOSITSCARDAMOUNT_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pNumber, const double amount);
typedef UhttStatus * (*UHTTWITHDRAWSCARDAMOUNT_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pNumber, const double amount);
typedef int (*UHTTGETSCARDREST_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pNumber, const char * pDate, double & rRest);
typedef int (*UHTTSTARTDATATRANSFER_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pName, int64 totalRawSize, int32 chunkCount);
typedef int (*UHTTTRANSFERDATA_PROC)(PPSoapClientSession & rSess, const char * pToken, int transferID, int chunkNumber, int64 rawChunkSize, char * pMime64Data);
typedef int (*UHTTFINISHTRANSFERDATA_PROC)(PPSoapClientSession & rSess, const char * pToken, int transferID);
typedef int (*UHTTADDFILEVERSION_PROC)(PPSoapClientSession & rSess, const char * pToken, int transferID, const char * pKey, const char * pLabel, const char * pMemo);
typedef UhttLocationPacket * (*UHTTGETLOCATIONBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int ID);
typedef UhttLocationPacket * (*UHTTGETLOCATIONBYCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef TSCollection <UhttLocationPacket> * (*UHTTGETLOCATIONLISTBYPHONE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pPhone);
typedef UhttStatus * (*UHTTCREATESTANDALONELOCATION_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttLocationPacket & rPack);
typedef int (*UHTTCREATEBILL_PROC)(PPSoapClientSession & rSess, const char * pToken, UhttBillPacket & rPack);
typedef TSCollection <UhttBillPacket> * (*UHTTGETBILL_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttBillFilter & rFilt);
typedef int (*UHTTSETQUOT_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttQuotPacket & rPack);
typedef TSCollection <UhttStatus> * (*UHTTSETQUOTLIST_PROC)(PPSoapClientSession & rSess, const char * pToken, const TSCollection <UhttQuotPacket> & rList);
typedef TSCollection <UhttQuotPacket> * (*UHTTGETQUOT_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttQuotFilter & rFilt);
typedef TSCollection <UhttGoodsRestListItem> * (*UHTTGETGOODSRESTLIST_PROC)(PPSoapClientSession & rSess, const char * pToken, int uhttGoodsID);
typedef TSCollection <UhttDCFileVersionInfo> * (*UHTTDCGETFILEVERSIONLIST_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pKey);

typedef UhttWorkbookItemPacket * (*UHTTGETWORKBOOKITEMBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int ID);
typedef UhttWorkbookItemPacket * (*UHTTGETWORKBOOKITEMBYCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef TSCollection <UhttWorkbookItemPacket> * (*UHTTGETWORKBOOKLISTBYPARENTCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef UhttDocumentPacket * (*UHTTGETWORKBOOKCONTENTBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int id);
typedef UhttStatus * (*UHTTCREATEWORKBOOKITEM_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttWorkbookItemPacket & rPack);
typedef UhttStatus * (*UHTTSETWORKBOOKCONTENTBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, UhttDocumentPacket & rPack);

typedef UhttStyloDevicePacket * (*UHTTGETSTYLODEVICEBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int ID);
typedef UhttStyloDevicePacket * (*UHTTGETSTYLODEVICEBYCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef UhttStatus * (*UHTTCREATESTYLODEVICE_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttStyloDevicePacket & rPack);

typedef UhttProcessorPacket * (*UHTTGETPROCESSORBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int id);
typedef UhttProcessorPacket * (*UHTTGETPROCESSORBYCODE_PROC)(PPSoapClientSession & rSess, const char * pToken, const char * pCode);
typedef UhttStatus * (*UHTTCREATEPROCESSOR_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttProcessorPacket & rPack);

typedef UhttTSessionPacket * (*UHTTGETTSESSIONBYID_PROC)(PPSoapClientSession & rSess, const char * pToken, int id);
typedef UhttTSessionPacket * (*UHTTGETTSESSIONBYUUID_PROC)(PPSoapClientSession & rSess, const char * pToken, const S_GUID & id);
typedef TSCollection <UhttTSessionPacket> * (*UHTTGETTSESSIONBYPRC_PROC)(PPSoapClientSession & rSess, const char * pToken, int prcID, const UhttTimestamp * pSince);
typedef UhttStatus * (*UHTTCREATETSESSION_PROC)(PPSoapClientSession & rSess, const char * pToken, const UhttTSessionPacket & rPack);

typedef TSCollection <UhttStatus> * (*UHTTSENDSMS_PROC)(PPSoapClientSession & rSess, const char * pToken, const TSCollection <UhttSmsPacket> & rPack);
//
// iSales PEPSI SOAP CLIENT
//
struct iSalesUOM {
	iSalesUOM();
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx);

	int    Code;     // 0 - �����, 1 - �������, 2 - �������� �������
    double Width;    // ������, ��
    double Height;   // ������, ��
    double Length;   // �����, ��
    double Netto;    // ����� �����, ��
    double Brutto;   // ����� ������, ��
    SString Barcode; // ��������
};

struct iSalesUOMCvt {
	iSalesUOMCvt();
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx);

	SString UomFrom;
	SString UomTo;
	double Rate;
};

struct iSalesGoodsPacket {
	iSalesGoodsPacket();
	iSalesGoodsPacket(const iSalesGoodsPacket & rS);
	iSalesGoodsPacket & FASTCALL Copy(const iSalesGoodsPacket & rS);
	iSalesGoodsPacket & FASTCALL operator = (const iSalesGoodsPacket & rS);
	double RecalcUnits(long uomFrom, long uomTo, double qtty) const;
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx);

    int    TypeOfProduct;
    SString OuterCode;
    SString NativeCode;
    SString Name;
    SString Abbr;
    double VatRate;
    int    UnitCode; // 0 - �����, 1 - �������, 2 - �������� �������
    int    Valid;    // 0 - active, 1 - passive
    SString CountryName;
    SString CLB;     // ���
    TSCollection <iSalesUOM> UomList;
    TSCollection <iSalesUOMCvt> CvtList;
    //
    SString ErrMsg;
};

struct iSalesVisit {
    SString Ident;
    int    Valid;	   // 0 - active, 1 - passive
    int    Freq;   // ������� (2 - ��� � 2 ������)
    int    DayOfWeek; // ���� ������ [1..7]
    LDATE  InitDate;
    SString Order; // ������� ���������
    SString OuterClientCode;
    SString InnerClientCode;
};

struct iSalesRoutePacket {
	iSalesRoutePacket();
	iSalesRoutePacket(const iSalesRoutePacket & rS);
	iSalesRoutePacket & FASTCALL Copy(const iSalesRoutePacket & rS);
	iSalesRoutePacket & FASTCALL operator = (const iSalesRoutePacket & rS);

    int    TypeOfRoute; // Pre sell - 0, Van sell - 1
    int    Valid;	    // 0 - active, 1 - passive
    SString Ident;
    SString Code;
    SString NativeAgentCode;
    TSCollection <iSalesVisit> VisitList;
    //
    SString ErrMsg;
};

struct iSalesCustomerPacket {
	SString Name;
	SString Title;
	SString ShortName;
	SString OuterCode;       // ��� ������� ��� ������ �������� � �����������
	SString NativeCode;      // ��� ������� ��� ������ �������� � ���
	SString OuterOwnerCode;  // ��� ��������� ������������� � �����������
	SString NativeOwnerCode; // ��� ��������� ������������� � ���
	SString INN;
	SString KPP;
	int    TaxScheme; // 0 - ���������� ���, 1 - ����
    //
    SString ErrMsg;
};

struct iSalesStockCountingItem {
	SString OuterCode;
	int   Type; // ��� ��������: 0 - ������, 1 - ����, 2 - ������
	int   UnitCode; // 0 - �����, 1 - �������, 2 - �������� �������
	double Qtty; // ���������� �� �������
};

struct iSalesStockCountingWhPacket {
	int   WhID;
	SString WhCode; // ������ ������
	TSCollection <iSalesStockCountingItem> Items;
};

struct iSalesPriceItem {
	SString OuterCode; // ��� ������ � �����������
	double Price;
};

struct iSalesPriceListPacket {
    int    PriceListID;
    StringSet OuterCliCodeList;
    TSCollection <iSalesPriceItem> Prices;
};

struct iSalesBillAmountEntry {
	iSalesBillAmountEntry();

	int    SetType;       // 0 - ��������, ������������ �� ������ ������� ���, 1 - ��������, ������� �.�. � ����������, 2 - ��������, ������� ����� ���������� � ����������
	double NetPrice;
	double GrossPrice;
	double NetSum;
	double DiscAmount;
	double DiscNetSum;
	double VatSum;
	double GrossSum;
	double DiscGrossSum;
};

struct iSalesBillItem {
	iSalesBillItem();

	int    LineN; // ����� ������
	SString OuterGoodsCode;  // Outer-��� ������
	SString NativeGoodsCode; // Native-��� ������
	SString Country; // ������ �������������
	SString CLB;     // ����� ���

	int    UnitCode;
	double Qtty;
	SString Memo; // DESC
	TSCollection <iSalesBillAmountEntry> Amounts;
};
//
// Descr: �������� ���������� ���������
//
struct iSalesBillRef {
	int    DocType; // ��� ���������� ���������
	SString Code;   // ����� ���������� ���������
	LDATETIME Dtm;  // ����/����� ���������� ���������
};
//
// Descr: �������������� ������� �������� ������ �� iSales
//
struct iSalesExtAttr {
	SString Name;
	SString Value;
};

struct iSalesBillPacket {
	iSalesBillPacket();

	long   NativeID;
	SString iSalesId;
	int    DocType; // 0 - ��������, 1 - �������� ����������, 2 - ���� �� ������, 3 - ������� �� ����������,
		// 4 - ������� �������� ����������, 5 - ������� �� ����, 6 - ��������� ��������/������, 7 - �������� �����������,
		// 8 - ������ ������������, 9 - �����������, 10 - �������� ����� ����� ���������, 11 - ��������/���������� �����,
		// 12 - ����� �� �������, 13 - ����� �� �������, 14 - ������� (������������� �����������)
	int    ExtDocType; // (������ ���� �����)
	int    Status; // 0 - ��������, 1 - �������
	SString Code;    // ����� ���������
	SString ExtCode; // ����� �������� ���������
	LDATETIME Dtm;    // ����/����� ��������� (���������� ������� dd.mm.yyyy hh:mm:ss)
	LDATETIME IncDtm; // ���� ��������� ��������� (������ ���� �����), ��� ������ - ���� ����������
	LDATETIME ExtDtm; // ����/����� �������� ��������� (���������� ������� dd.mm.yyyy hh:mm:ss)
	LDATETIME CreationDtm; // ����/����� �������� ��������� (���������� ������� dd.mm.yyyy hh:mm:ss)
	LDATETIME LastUpdDtm;  // ����/����� ���������� ��������� ��������� (���������� ������� dd.mm.yyyy hh:mm:ss)
	LDATE   DueDate;  // ���� ������
	SString ShipFrom; // ��� ���������������� (iSales ��� ������������� ��� �������� �� �������, Native-��� ������� ��� ��������)
	SString ShipTo;   // ��� ��������������� (Native-��� ������� (����� ��������) ��� ��������� ����������, iSales ��� ������������� ��� ��������)
    SString SellerCode; // ��� �������� (iSales ��� ������������� ��� �������� �� �������, Native-��� ������� ��� ��������)
	SString PayerCode;  // ��� ����������� (Native-��� ������� ��� �������� �� �������, iSales ��� ������������� ��� ��������)
	SString Memo;      // ����������
	SString SrcLocCode;  // ��� ������-�����������
	SString DestLocCode; // ��� ������-����������
	SString AgentCode;   // Native-��� ������
	SString AuthId;      // �������� ������������ (Native-���), ��� ��������������� �������� � iSales
	SString EditId;      // �������� ������������ (Native-���), ��� ������� �������� � iSales
	SString ErrMsg;      // ��������� �� ������, ���������� �� �������
	TSCollection <iSalesBillItem> Items;
	TSCollection <iSalesBillAmountEntry> Amounts;
	TSCollection <iSalesBillRef> Refs;
	TSCollection <iSalesExtAttr> Attrs; // @v9.3.1
};

struct iSalesBillDebt {
	SString iSalesId;
	int   DocType;
	SString Code;
	LDATETIME Dtm;
	SString PayerCode;  // ��� ����������� (Native-��� ������� ��� �������� �� �������, iSales ��� ������������� ��� ��������)
	double Amount;
	double Debt;
	//
	SString ErrMsg;
};

struct iSalesTransferStatus {
	enum {
		ifcUnkn = 0,
		ifcOrder,      // ORDS
		ifcInvoice,    // DOCL
		ifcReceipt     // DOCS
	};
	int    Ifc;
	SString Ident; // ������������� ���������� �� ����������� ������� ������� (����������� �������� ���������� iSalesTransferStatus)
};

typedef TSCollection <iSalesGoodsPacket> * (*ISALESGETGOODSLIST_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const DateRange * pPeriod);
typedef TSCollection <iSalesRoutePacket> * (*ISALESGETROUTELIST_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const DateRange * pPeriod);
typedef TSCollection <iSalesBillPacket>  * (*ISALESGETORDERLIST_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const DateRange * pPeriod, int inclProcessedItems);
typedef TSCollection <iSalesBillPacket>  * (*ISALESGETRECEIPTLIST_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const DateRange * pPeriod, int inclProcessedItems);
typedef TSCollection <iSalesBillDebt>    * (*ISALESGETUNCLOSEDBILLLIST_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const DateRange * pPeriod);
typedef SString * (*ISALESPUTSTOCKCOUNTING_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const TSCollection <iSalesStockCountingWhPacket> * pItems);
typedef SString * (*ISALESPUTPRICES_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const TSCollection <iSalesPriceListPacket> * pItems);
typedef SString * (*ISALESPUTBILLS_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const TSCollection <iSalesBillPacket> * pItems, uint maxItems);
typedef SString * (*ISALESPUTTRANSFERSTATUS_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const TSCollection <iSalesTransferStatus> * pItems);
typedef SString * (*ISALESPUTDEBTSETTLEMENT_PROC)(PPSoapClientSession & rSess, const char * pUser, const char * pPassw, const TSCollection <iSalesBillDebt> * pItems);
//
//
//
struct SapEfesCallHeader {
	SapEfesCallHeader();

	const char * P_SalesOrg;
	const char * P_Wareh;
	long   SessionID;
};

enum SapEfesUnitType {
    sapefesUnitItem = 1,  // "ST"
    sapefesUnitPack,      // "PCG"
    sapefesUnitMultipack, // "MPK"
    spaefesUnitLiter      // "L"
};

struct SapEfesBillItem {
    int    PosN;   // ����� �������
    int    UnitType;
    int    BaseUnitType;
    double Qtty;   // ���������� � ��������� ������� ���������
    double QtyN;   // ��������� ������������ ��������� ������� �� ��������� � �������
    double QtyD;   // ����������� ������������ ��������� ������� �� ��������� � �������
	double Amount; // ����� ��� ��� �� ������
    SString PosType;
    SString GoodsCode;
    SString Currency;
};

struct SapEfesContractor {
    SString Code;
    SString Name;
    SString Addr;
    SString City;
    SString Region;
    SString ZIP;
};

struct SapEfesOrder {
	SapEfesOrder();

	LDATETIME Date; // ����� �������� ���������
	LDATE  DueDate; // ���� ��������
	double Amount;  // ����� ������ ��� ���
    SString Code;
    SString DocType;
    SString Status;
    SString PaymTerm;
    SString ShipCond;
    SString ShipCondDescr;
    SString SalesOrg;
    SString DistrChannel;
    SString Division;
    SString Warehouse;
    SString TerrIdent;
    SString Currency;
    SString Memo;
    SapEfesContractor Buyer;
    SapEfesContractor DlvrLoc;
    TSCollection <SapEfesBillItem> Items;
};

struct SapEfesLogMsg {
	enum {
		tS = 1,
		tW = 2,
		tE = 3
	};
	int    MsgType;
	SString Msg;
};

struct SapEfesBillStatus {
    SString NativeCode;
    SString Code;
    SString Status;
	TSCollection <SapEfesLogMsg> MsgList;
};

struct SapEfesBillPacket {
	SapEfesBillPacket();

	enum {
		fHasOrderRef = 0x0001,
		fReturn      = 0x0002
	};
	enum {
		tRetail = 1, // ZDSD
		tReturn,     // ZRET
		tBonus,      // ZDS1
		tBonusEfes   // ZDS2
	};
	long   NativeID; // ������������� ��������� � ����������� �� (���������� �� ����������)
	int    Flags;
    int    DocType;
    LDATE  Date;
    LDATE  DueDate;
    SString NativeCode;
    SString OrderCode;
    SString BuyerCode;
    SString DlvrLocCode;
    SString Memo;
    TSCollection <SapEfesBillItem> Items;
};

struct SapEfesGoodsReportEntry {
	LDATE  Dt;
	int    UnitType;
	double Qtty;
	SString DlvrLocCode;
    SString GoodsCode;
};

struct SapEfesDebtReportEntry {
	SapEfesDebtReportEntry();

	long   NativeArID; // @anchor(strictly the first member) @v9.5.2 �� ������ � ����� ���� ������
	double Debt;
	double CreditLimit;
	long   PayPeriod;
	long   DebtDelayDays; // ������������� � ����
	SString BuyerCode;
	SString Memo;
};

struct SapEfesDebtDetailReportEntry {
	SapEfesDebtDetailReportEntry();

	long   NativeArID; // @anchor(strictly the first member) @v9.5.2 �� ������ � ����� ���� ������
	LDATE  BillDate;
	LDATE  PaymDate;
	double Amount;
	double Debt;
	SString BuyerCode;
	SString NativeBillCode;
};

typedef TSCollection <SapEfesOrder> * (*EFESGETSALESORDERSYNCLIST_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const DateRange * pPeriod, int repeat, const char * pDocNumberList);
typedef int (*EFESSETSALESORDERSTATUSSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesBillStatus> * pItems);
typedef TSCollection <SapEfesBillStatus> * (*EFESSETDELIVERYNOTESYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesBillPacket> * pItems);
typedef TSCollection <SapEfesBillStatus> * (*EFESCANCELDELIVERYNOTESYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SString> * pItems);
typedef TSCollection <SapEfesLogMsg> * (*EFESSETDAILYSTOCKREPORTSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesGoodsReportEntry> * pItems);
typedef TSCollection <SapEfesLogMsg> * (*EFESSETMTDPRODUCTREPORTSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesGoodsReportEntry> * pItems);
typedef TSCollection <SapEfesLogMsg> * (*EFESSETMTDOUTLETSREPORTSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesGoodsReportEntry> * pItems);
typedef TSCollection <SapEfesLogMsg> * (*EFESSETDEBTSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesDebtReportEntry> * pItems);
typedef TSCollection <SapEfesLogMsg> * (*EFESSETDEBTDETAILSYNC_PROC)(PPSoapClientSession & rSess, const SapEfesCallHeader & rH, const TSCollection <SapEfesDebtDetailReportEntry> * pItems);
//
//
//
struct VetisBusinessEntity;

struct VetisErrorEntry {
	SString Item;
	SString Code;
	SString Qualifier;
};

struct VetisEntityList {
	VetisEntityList() : Count(0), Total(0), Offset(0)
	{
	}
	int    Count;
	int64  Total;
	int    Offset;
};

struct VetisGenericEntity {
	VetisGenericEntity()
	{
		Uuid.SetZero();
	}
	S_GUID Uuid;
};

struct VetisEnterpriseActivity : public VetisGenericEntity {
	SString Name;
};

struct VetisGenericVersioningEntity : public VetisGenericEntity {
	VetisGenericVersioningEntity() : VetisGenericEntity(), Flags(0), Status(0), CreateDate(ZERODATETIME), UpdateDate(ZERODATETIME)
	{
		Guid.SetZero();
		Previous.SetZero();
		Next.SetZero();
	}
	S_GUID Guid;
	enum {
		fActive = 0x0001,
		fLast   = 0x0002
	};
	long   Flags;
	int    Status; // VersionStatus
	LDATETIME CreateDate;
	LDATETIME UpdateDate;
	S_GUID Previous;
	S_GUID Next;
};

struct VetisNamedGenericVersioningEntity : public VetisGenericVersioningEntity {
	SString Name;
};

struct VetisCountry {
	SString Name;
	SString FullName;
	SString EnglishName;
	SString Code;
	SString Code3;
};

struct VetisFederalDistrict {
	SString FullName;
	SString ShortName;
	SString Abbreviation;
};

struct VetisAddressObjectView {
	VetisAddressObjectView() : Flags(0)
	{
		CountryGUID.SetZero();
	}
	SString Name;
	SString EnglishName;
	SString View;
	SString RegionCode;
	SString Type;
	S_GUID CountryGUID;
	enum {
		fHasStreets = 0x0001
	};
	long   Flags;
};

struct VetisDistrict : public VetisAddressObjectView {
	VetisDistrict()
	{
		RegionGUID.SetZero();
	}
	S_GUID RegionGUID;
};

struct VetisLocality : public VetisAddressObjectView {
	VetisLocality()
	{
		RegionGUID.SetZero();
		DistrictGUID.SetZero();
		CityGUID.SetZero();
	}
	S_GUID RegionGUID;
	S_GUID DistrictGUID;
	S_GUID CityGUID;
};

struct VetisStreet : public VetisAddressObjectView {
	VetisStreet()
	{
		LocalityGUID.SetZero();
	}
	S_GUID LocalityGUID;
};

struct VetisAddress {
	VetisCountry Country;
	VetisFederalDistrict FederalDistrict;
	VetisAddressObjectView Region;
	VetisDistrict District;
	VetisLocality Locality;
	VetisLocality SubLocality;
	VetisStreet Street;
	SString House;
	SString Building;
	SString Room;
	SString PposIndex;
	SString PostBox;
	SString AdditionalInfo;
	SString AddressView;
	SString EnAddressView;
};

struct VetisOrganization {
	SString ID;
	SString Name;
	VetisAddress Address;
};

struct VetisDocument {
	VetisDocument() : IssueDate(ZERODATE), DocumentType(0)
	{
	}
	SString Name;
	SString Form;
	SString IssueSeries;
	SString IssueNumber;
	LDATE  IssueDate;
	int    DocumentType;
	VetisOrganization Issuer;
	SString For_; 
	SString Qualifier; // ?
};

struct VetisUserAuthority {
	VetisUserAuthority() : Granted(0)
	{
	}
	SString ID;
	SString Name;
	int    Granted; // bool
};

struct VetisUser {
	VetisUser();
	~VetisUser();
	//class ns7__WorkingAreaList *workingAreaList;	/* optional element of type ns7:WorkingAreaList */
	SString Login;
	SString Fio;
	LDATE  BirthDate;
	VetisDocument Identity;
	SString Snils;
	SString Phone;
	SString Email;
	SString WorkEmail;
	//
	int    UnionUser;
	VetisOrganization * P_Organization;
	VetisBusinessEntity * P_BusinessEntity;
	//
	SString Post;
	enum {
		fEnabled    = 0x0001,
		fNonExpired = 0x0002,
		fNonLocked  = 0x0004
	};
	long   Flags;
	TSCollection <VetisUserAuthority> AuthorityList;
};

struct VetisEnterpriseOfficialRegistration {
	VetisEnterpriseOfficialRegistration();
	~VetisEnterpriseOfficialRegistration();

	SString ID; // GRNType
	VetisBusinessEntity * P_BusinessEntity;
	SString Kpp;
};

struct VetisEnterpriseActivityList : public VetisEntityList {
	TSCollection <VetisEnterpriseActivity> Activity;
};

struct VetisEnterprise : public VetisNamedGenericVersioningEntity {
	VetisEnterprise();
	~VetisEnterprise();
	SString EnglishName;
	int    Type; // EnterpriseType
	StringSet NumberList; // EnterpriseNumberList
	VetisAddress Address;
	VetisEnterpriseActivityList ActivityList;
	VetisBusinessEntity * P_Owner;
	TSCollection <VetisEnterpriseOfficialRegistration> OfficialRegistration;
};

struct VetisBusinessEntity_activityLocation {
	StringSet GlobalID; // ������ GLN
	VetisEnterprise Enterprise;
};

struct VetisBusinessEntity : public VetisNamedGenericVersioningEntity {
	VetisBusinessEntity() : VetisNamedGenericVersioningEntity(), Type(0), IncorporationForm(0)
	{
	}
	int    Type; // BusinessEntityType
	int    IncorporationForm; // IncorporationForm
	SString FullName;
	SString Fio;
	SString Passport;
	SString Inn;
	SString Kpp;
	SString Ogrn;
	VetisAddress JuridicalAddress;
	TSCollection <VetisBusinessEntity_activityLocation> ActivityLocationList;
};

struct VetisListOptions {
	VetisListOptions() : Count(0), Offset(0)
	{
	}
	uint   Count;
	uint   Offset;
};

struct VetisProduct : public VetisNamedGenericVersioningEntity {
	VetisProduct() : VetisNamedGenericVersioningEntity(), ProductType(0)
	{
	}
	SString Code;
	SString EnglishName;
	int    ProductType; // ProductType
};

struct VetisSubProduct : public VetisNamedGenericVersioningEntity {
	VetisSubProduct() : VetisNamedGenericVersioningEntity()
	{
		ProductGuid.SetZero();
	}
	SString Code;
	SString EnglishName;
	S_GUID ProductGuid;
};

struct VetisUnit : public VetisNamedGenericVersioningEntity {
	VetisUnit() : VetisNamedGenericVersioningEntity(), Factor(0)
	{
		CommonUnitGuid.SetZero();
	}
	SString FullName;
	S_GUID CommonUnitGuid;
	int   Factor;
};

struct VetisPackingType : public VetisNamedGenericVersioningEntity {
	VetisPackingType() : VetisNamedGenericVersioningEntity(), GlobalID(0)
	{
	}
	int    GlobalID; // PackingCodeType
};

struct VetisPackaging {
	VetisPackaging() : Quantity(0), Volume(0.0)
	{
	}
	VetisPackingType PackagingType;
	int    Quantity;
	double Volume;
	VetisUnit Unit;
};

struct VetisProductMarks {
	VetisProductMarks() : Cls(0)
	{
	}
	SString Item;
	int   Cls; // ProductMarkingClass
};

struct VetisPackage {
	VetisPackage() : Level(0), Quantity(0)
	{
	}
	int    Level; // PackageLevelType
	VetisPackingType PackingType;
	int    Quantity;
	TSCollection <VetisProductMarks> ProductMarks;
};

struct VetisProductItem : public VetisNamedGenericVersioningEntity {
	VetisProductItem() : VetisNamedGenericVersioningEntity(), ProductType(0), Flags(0)
	{
	}
	SString GlobalID;
	SString Code;
	int    ProductType; // ProductType
	VetisProduct Product;
	VetisSubProduct SubProduct;
	enum {
		fCorrespondsToGost = 0x0001,
		fIsPublic = 0x0002
	};
	long   Flags;
	SString Gost;
	VetisBusinessEntity Producer;
	VetisBusinessEntity TmOwner;
	TSCollection <VetisEnterprise> Producing;
	VetisPackaging Packaging;
};

struct VetisGoodsDate {
	VetisGoodsDate() : FirstDate(ZERODATETIME), SecondDate(ZERODATETIME)
	{
	}
	LDATETIME FirstDate;
	LDATETIME SecondDate;
	SString InformalDate;
};

struct VetisProducer {
	VetisProducer() : Role(0)
	{
	}
	VetisEnterprise Enterprise;
	int    Role; // EnterpriseRole
};

struct VetisBatchOrigin {
	VetisProductItem ProductItem;
	VetisCountry Country;
	TSCollection <VetisProducer> Producer;
};

struct VetisBatch {
	VetisBatch() : ProductType(0), Volume(0.0), Flags(0), P_Owner(0)
	{
	}
	~VetisBatch()
	{
		delete P_Owner;
	}
	int    ProductType; // ProductType
	VetisProduct Product;
	VetisSubProduct SubProduct;
	VetisProductItem ProductItem;
	double Volume;
	VetisUnit Unit;
	VetisGoodsDate DateOfProduction;
	VetisGoodsDate ExpiryDate;
	StringSet BatchIdList;
	enum {
		fPerishable    = 0x0001,
		fLowGradeCargo = 0x0002
	};
	long   Flags;
	VetisBatchOrigin Origin;
	TSCollection <VetisPackage> PackageList;
	VetisBusinessEntity * P_Owner;
};

struct VetisBusinessMember {
	VetisBusinessEntity BusinessEntity;
	VetisEnterprise Enterprise;
	SString GlobalID;
};

struct VetisCertifiedBatch {
	VetisBusinessMember Producer;
	VetisBatch Batch;
};

struct VetisTransportNumber {
	SString ContainerNumber;
	SString WagonNumber;
	SString VehicleNumber;
	SString TrailerNumber;
	SString ShipName;
	SString FlightNumber;
};

struct VetisTransportInfo {
	VetisTransportInfo() : TransportType(0)
	{
	}
	int    TransportType; // TransportType
	VetisTransportNumber TransportNumber;
};

struct VetisLocation {
	SString Name;
	SString Address;
};

struct VetisShipmentRoutePoint : public VetisGenericEntity {
	VetisShipmentRoutePoint() : UnionShipmentRoutePoint(0), Flags(0), P_Location(0), P_Enterprise(0), P_NextTransport(0)
	{
	}
	~VetisShipmentRoutePoint()
	{
		delete P_NextTransport;
		delete P_Location;
		delete P_Enterprise;
	}
	SString SqnId;
	//
	int   UnionShipmentRoutePoint;
	VetisLocation * P_Location;
	VetisEnterprise * P_Enterprise;
	//
	enum {
		fTransShipment = 0x0001
	};
	long   Flags;
	VetisTransportInfo * P_NextTransport;
};

struct VetisCertifiedConsignment {
	VetisCertifiedConsignment() : TransportStorageType(0)
	{
	}
	VetisBusinessMember Consignor;
	VetisBusinessMember Consignee;
	VetisBusinessEntity Broker;
	VetisTransportInfo TransportInfo;
	int    TransportStorageType; // TransportationStorageType
	TSCollection <VetisShipmentRoutePoint> RoutePointList;
	VetisBatch Batch;
};

struct VetisPurpose : public VetisNamedGenericVersioningEntity {
	enum {
		fForSubstandard = 0x0001
	};
	long   Flags;
};

struct VetisVeterinaryEvent {
	VetisVeterinaryEvent() : Type(0), ActualDateTime(ZERODATETIME), UnionVeterinaryEvent(0), P_Location(0)
	{
	}
	SString ID;
	SString Name;
	int   Type; // VeterinaryEventType
	LDATETIME ActualDateTime;
	int   UnionVeterinaryEvent;
	union {
		VetisLocation * P_Location;
		VetisEnterprise * P_Enterprise;
	};
	VetisOrganization Operator;
	TSCollection <VetisDocument> ReferencedDocumentList;
	SString Notes;
};

typedef VetisNamedGenericVersioningEntity VetisIndicator;
typedef VetisNamedGenericVersioningEntity VetisAnimalDisease;
typedef VetisNamedGenericVersioningEntity VetisResearchMethod;

struct VetisLaboratoryResearchEvent : public VetisVeterinaryEvent {
	VetisLaboratoryResearchEvent() : VetisVeterinaryEvent(), Union_LaboratoryResearchEvent(0), Result(0)
	{
	}
	StringSet BatchIdList;
	SString ExpertiseID;
	int   Union_LaboratoryResearchEvent;
	VetisNamedGenericVersioningEntity IndOrDis; // VetisIndicator || VetisAnimalDisease
	VetisResearchMethod Method;
	int    Result; // ResearchResult
	SString Conclusion;
};

struct VetisQuarantineEvent : public VetisVeterinaryEvent {
	SString Duration;
};

struct VetisMedicinalDrug {
	SString ID;
	SString Name;
	SString Series;
	VetisBusinessMember Producer;
};

struct VetisAnimalMedicationEvent : public VetisVeterinaryEvent {
	VetisAnimalMedicationEvent() : VetisVeterinaryEvent(), EffectiveBeforeDate(ZERODATETIME)
	{
	}
	VetisAnimalDisease Disease;
	VetisMedicinalDrug MedicinalDrug;
	LDATETIME EffectiveBeforeDate;
};

struct VetisRegionalizationCondition : public VetisGenericVersioningEntity {
	VetisRegionalizationCondition() : VetisGenericVersioningEntity(), Flags(0)
	{
	}
	SString ReferenceNumber;
	SString Text;
	enum {
		fStrict = 0x0001
	};
	long   Flags;
	TSCollection <VetisAnimalDisease> RelatedDiseaseList;
};

struct VetisRegionalizationClause {
	VetisRegionalizationCondition Condition;
	SString Text;
};

struct VetisVeterinaryAuthentication {
	VetisVeterinaryAuthentication() : Flags(0), CargoExpertized(0), AnimalSpentPeriod(0)
	{
	}
	VetisPurpose Purpose;
	enum {
		fCargoInspected = 0x0001
	};
	long   Flags;
	int    CargoExpertized; // ResearchResult
	SString LocationProsperity; 
	int    AnimalSpentPeriod; // AnimalSpentPeriod
	SString MonthsSpent;
	TSCollection <VetisLaboratoryResearchEvent> LaboratoryResearchList;
	VetisQuarantineEvent Quarantine;
	TSCollection <VetisAnimalMedicationEvent> ImmunizationList;
	TSCollection <VetisVeterinaryEvent> VeterinaryEventList;
	TSCollection <VetisRegionalizationClause> R13nClauseList;
	SString SpecialMarks;
};

struct VetisReferencedDocument : public VetisDocument {
	VetisReferencedDocument() : VetisDocument(), RelationshipType(0)
	{
	}
	int    RelationshipType; // ReferenceType
};

struct VetisVetDocumentStatusChange {
	VetisVetDocumentStatusChange() : Status(0), ActualDateTime(ZERODATETIME)
	{
	}
	int    Status; // VetDocumentStatus
	VetisUser SpecifiedPerson;
	LDATETIME ActualDateTime;
	SString Reason;
};

struct VetisVetDocument : public VetisDocument {
	VetisVetDocument() : VetisDocument(), VetDForm(0), VetDType(0), VetDStatus(0), Flags(0), LastUpdateDate(ZERODATETIME),
		UnionVetDocument(0), P_CertifiedBatch(0), P_CertifiedConsignment(0)
	{
	}
	~VetisVetDocument()
	{
		delete P_CertifiedBatch;
		delete P_CertifiedConsignment;
	}
	int    VetDForm;   // VetDocumentForm
	int    VetDType;   // VetDocumentType
	int    VetDStatus; // VetDocumentStatus
	enum {
		fFinalized = 0x0001
	};
	long   Flags;
	LDATETIME LastUpdateDate;
	//
	int    UnionVetDocument;
	VetisCertifiedBatch * P_CertifiedBatch;
	VetisCertifiedConsignment * P_CertifiedConsignment;
	//
	VetisVeterinaryAuthentication Authentication;
	SString PrecedingVetDocuments;
	TSCollection <VetisReferencedDocument> ReferencedDocumentList;
	TSCollection <VetisVetDocumentStatusChange> StatusChangeList;
};

struct VetisStockEntryEventList {
	TSCollection <VetisLaboratoryResearchEvent> LaboratoryResearchList;
	TSCollection <VetisQuarantineEvent> QuarantineList;
	TSCollection <VetisAnimalMedicationEvent> ImmunizationList;
	TSCollection <VetisVeterinaryEvent> VeterinaryEventList;
};

struct VetisStockEntry : public VetisGenericVersioningEntity {
	SString EntryNumber;
	VetisBatch Batch;
	VetisStockEntryEventList VetEventList;
	TSCollection <VetisVetDocument> VetDocumentList;
};

struct VetisStockEntrySearchPattern : public VetisStockEntry {
	VetisStockEntrySearchPattern() : VetisStockEntry(), BlankFilter(0)
	{
	}
	int    BlankFilter; // StockEntryBlankFilter
};

class VetisApplicationData {
public:
	enum {
		signNone = 0,
		signGetStockEntryListRequest
	};
	explicit VetisApplicationData(int sign)
	{
		Sign = sign;
	}
	virtual ~VetisApplicationData()
	{
	}

	int    Sign;
	//
	SString LocalTransactionId;
	VetisUser Initiator;
	SString SessionToken;
};

class VetisGetStockEntryListRequest : public VetisApplicationData {
public:
	VetisGetStockEntryListRequest() : VetisApplicationData(signGetStockEntryListRequest)
	{
		EnterpriseGuid.SetZero();
	}
	VetisListOptions ListOptions;
	S_GUID EnterpriseGuid;
	VetisStockEntrySearchPattern SearchPattern;
};

struct VetisApplicationBlock {
	VetisApplicationBlock();
	~VetisApplicationBlock();

	enum {
		detUndef = 0,
		detGetStockEntryListReq = 1
	};

	enum {
		appstUndef     = -1,
		appstAccepted  = 0,
		appstInProcess = 1,
		appstCompleted = 2,
		appstRejected  = 3
	};
	int    ApplicationStatus;
	int    Func;
	int64  LocalTransactionId;
	SString ServiceId;
	S_GUID ApplicationId;
	S_GUID IssuerId;
	S_GUID EnterpriseId;
	LDATETIME IssueDate;
	LDATETIME RcvDate;
	LDATETIME PrdcRsltDate;

	//SString Data;
	//SString Result;
	TSCollection <VetisErrorEntry> ErrList;
	//
	VetisGetStockEntryListRequest * P_GselReq;
};

typedef VetisApplicationBlock * (* VETIS_SUBMITAPPLICATIONREQUEST_PROC)(PPSoapClientSession & rSess, const char * pApiKey, const VetisApplicationBlock & rBlk);
typedef VetisApplicationBlock * (* VETIS_RECEIVEAPPLICATIONRESULT_PROC)(PPSoapClientSession & rSess, const char * pApiKey, const S_GUID & rIssuerId, const S_GUID & rApplicationId);
