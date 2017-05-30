// DBTABLEC.CPP
// Copyright (c) Sobolev A. 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2013, 2014, 2015, 2016, 2017
// @codepage windows-1251
// ������ � ������� DBTable, �� ��������� �� ���������� DBMS
//
#include <db.h>
#pragma hdrstop
//
//
//
DBTable * FASTCALL _GetTable(int handle)
{
	return DBS.GetTLA().GetTableEntry(handle);
}
//
//
// static members of DBTable {
const char * DBTable::CrTempFileNamePtr = (const char *)0x0003;

void FASTCALL DBTable::InitErrFileName(const char * pFileName)
{
	DBS.GetTLA().InitErrFileName(pFileName);
}

const char * SLAPI DBTable::GetLastErrorFileName()
{
	return DBS.GetConstTLA().GetLastErrFileName();
}

void SLAPI DBTable::InitErrFileName()
{
	DBS.GetTLA().InitErrFileName(OpenedFileName);
}
// } static members of DBTable
//
//
DBRowId::DBRowId()
{
	memzero(S, sizeof(S));
}

int DBRowId::IsLong() const
{
	return BIN(PTR32(S)[0] && PTR32(S)[1]);
}

DBRowId::operator RECORDNUMBER() const
{
	return B;
}

DBRowId & FASTCALL DBRowId::operator = (RECORDNUMBER n)
{
	B = n;
	PTR32(S)[1] = 0;
	return *this;
}

void DBRowId::SetZero()
{
	memzero(S, sizeof(S));
}

void DBRowId::SetMaxVal()
{
	memset(S, 0xff, sizeof(S)-1);
	S[sizeof(S)-1] = 0;
}

SString & FASTCALL DBRowId::ToStr(SString & rBuf) const
{
	if(B != 0)
		if(PTR32(S)[1] == 0)
			(rBuf = 0).Cat(B);
		else
			rBuf = (const char *)S;
	else
		rBuf = 0;
	return rBuf;
}

int FASTCALL DBRowId::FromStr(const char * pStr)
{
	int    ok = -1;
	const  size_t len = sstrlen(pStr);
	SetZero();
	if(len) {
		for(uint i = 0; i < len; i++) {
			if(!isdec(pStr[i])) {
				//
				// � ������ ���� �� �������� ������: �������� �� ������ ��� ��������� �������������
				//
				strnzcpy((char *)S, pStr, sizeof(S));
				ok = 2;
				break;
			}
		}
		if(ok < 0) {
			//
			// � ������ ��� ������� �������� (��. ����): �������� �� ��� ����������� �����
			//
			strtoulong(pStr, &B);
			PTR32(S)[1] = 0;
			ok = 1;
		}
	}
	return ok;
}
//
//
//
//
const uint32 SLobSignature[4] = { 0x2efc, 0xd421, 0x426c, 0xee07 };

int SLob::IsStructured() const
{
	return BIN(memcmp(Buf.H.Signature, SLobSignature, sizeof(SLobSignature)) == 0);
}

int SLob::SetStructured()
{
	if(!IsStructured()) {
		THISZERO();
		memcpy(Buf.H.Signature, SLobSignature, sizeof(SLobSignature));
		return 1;
	}
	else
		return -1;
}

int FASTCALL SLob::Init(uint32 descriptor)
{
	DestroyPtr();
	THISZERO();
	if(descriptor) {
		SetStructured();
		Buf.H.H = descriptor;
	}
	return 1;
}

int SLob::Empty()
{
	DestroyPtr();
	THISZERO();
	SetStructured();
	return 1;
}

int SLob::EnsureUnstructured()
{
	int    ok = -1;
	if(IsStructured()) {
		memzero(Buf.H.Signature, sizeof(Buf.H.Signature));
		ok = 1;
	}
	return ok;
}

int FASTCALL SLob::InitPtr(uint32 sz)
{
	int    ok = -1;
	DestroyPtr();
	if(sz) {
		SetStructured();
		Buf.H.H = (uint32)SAlloc::M(sz);
		if(!Buf.H.H) {
			Buf.H.PtrSize = 0;
			ok = 0;
		}
		else {
			Buf.H.PtrSize = sz;
			Buf.H.Flags |= hfPtr;
			ok = 1;
		}
	}
	return ok;
}

int SLob::IsPtr() const
{
	return BIN(IsStructured() && Buf.H.Flags & hfPtr);
}

int SLob::DestroyPtr()
{
	int    ok = -1;
	if(IsStructured()) {
		if(Buf.H.Flags & hfPtr) {
			SAlloc::F((void *)Buf.H.H);
			Buf.H.H = 0;
			Buf.H.PtrSize = 0;
			Buf.H.Flags &= ~hfPtr;
			ok = 1;
		}
		THISZERO();
	}
	return ok;
}

void * SLob::GetRawDataPtr()
{
	if(IsStructured())
		return (Buf.H.Flags & hfPtr) ? (void *)Buf.H.H : 0;
	else
		return Buf.B;
}

size_t SLob::GetPtrSize() const
{
	if(IsStructured())
		return (Buf.H.Flags & hfPtr) ? Buf.H.PtrSize : 0;
	else
		return 0;
}

int SLob::Serialize(int dir, size_t flatSize, uint8 * pInd, SBuffer & rBuf)
{
	int    ok = 1;
	if(dir > 0) {
		if(IsPtr()) {
			void * p = GetRawDataPtr();
			ulong  sz = Buf.H.PtrSize;
			*pInd = 2;
			rBuf.Write(sz);
			rBuf.Write(p, sz);
		}
		else if(ismemzero(Buf.B, flatSize))
			*pInd = 1;
		else {
			rBuf.Write(Buf.B, flatSize);
			*pInd = 0;
		}
	}
	else if(dir < 0) {
		if(*pInd == 1) {
			DestroyPtr();
			memzero(Buf.B, flatSize);
		}
		else if(*pInd == 2) {
			ulong  sz = 0;
			rBuf.Read(sz);
			InitPtr(sz);
			rBuf.Read(GetRawDataPtr(), sz);
		}
		else {
			DestroyPtr();
			rBuf.Read(Buf.B, flatSize);
		}
	}
	return ok;
}
//
//
//
DBLobBlock::DBLobBlock() : SArray(sizeof(DBLobItem))
{
}

int FASTCALL DBLobBlock::SearchPos(uint fldIdx, uint * pPos) const
{
	int    ok = -1;
	uint   pos = 0;
	const  uint c = getCount();
	if(c) {
		for(uint i = 0; i < c; i++) {
			if(((DBLobItem *)at(i))->FldN == fldIdx) {
				pos = i;
				ok = 1;
				break;
			}
		}
		if(ok <= 0)
			ok = 0;
	}
	ASSIGN_PTR(pPos, pos);
	return ok;
}

int DBLobBlock::SetSize(uint fldIdx, size_t sz)
{
	uint   pos = 0;
	int    ok = SearchPos(fldIdx, &pos);
	if(ok > 0)
		((DBLobItem *)at(pos))->Size = sz;
	return ok;
}

int DBLobBlock::GetSize(uint fldIdx, size_t * pSz) const
{
	size_t sz = 0;
	uint   pos = 0;
	int    ok = SearchPos(fldIdx, &pos);
	if(ok > 0)
		sz = ((DBLobItem *)at(pos))->Size;
	ASSIGN_PTR(pSz, sz);
	return ok;
}

int DBLobBlock::SetLocator(uint fldIdx, uint32 loc)
{
	uint   pos = 0;
	int    ok = SearchPos(fldIdx, &pos);
	if(ok > 0)
		((DBLobItem *)at(pos))->Loc = loc;
	return ok;
}

int DBLobBlock::GetLocator(uint fldIdx, uint32 * pLoc) const
{
	size_t loc = 0;
	uint   pos = 0;
	int    ok = SearchPos(fldIdx, &pos);
	if(ok > 0)
		loc = ((DBLobItem *)at(pos))->Loc;
	ASSIGN_PTR(pLoc, loc);
	return ok;
}
//
//
//
DBTable::SelectStmt::SelectStmt(DbProvider * pDb, const char * pText, int idx, int sp, int sf) : SSqlStmt(pDb, pText)
{
	Idx = idx;
	Sp = sp;
	Sf = sf;
}

int SLAPI DBTable::SetStmt(SelectStmt * pStmt)
{
	if(pStmt) {
		if(pStmt != P_Stmt) {
			ZDELETE(P_Stmt);
			P_Stmt = pStmt;
		}
	}
	else {
		ZDELETE(P_Stmt);
	}
	return 1;
}

int SLAPI DBTable::ToggleStmt(int release)
{
	if(release)
		ZDELETE(P_OppStmt);
	else {
		SelectStmt * p_temp = P_OppStmt;
		P_OppStmt = P_Stmt;
		P_Stmt = p_temp;
	}
	return 1;
}

DBTable::SelectStmt * SLAPI DBTable::GetStmt()
{
	return P_Stmt;
}

int (*DBTable::OpenExceptionProc)(const char * pFileName, int btrErr) = 0; // @global

int SLAPI DBTable::Init(DbProvider * pDbP)
{
	index  = 0;
	buf    = 0;
	bufLen = 0;
	State  = 0;
	FixRecSize = 0;
	P_Db = NZOR(pDbP, CurDict);
	P_Stmt = 0;
	P_OppStmt = 0;

	handle  = 0;
	flags   = 0;
	tableID = 0;
	ownrLvl = 0;
	tableName[0] = 0;
	fileName = 0;
	indexes.setTableRef(offsetof(DBTable, indexes));
	PageSize = 0;
	LastLockedRow.SetZero(); // @v8.1.4
	return 1;
}

SLAPI DBTable::DBTable()
{
	Init(0);
}

SLAPI DBTable::DBTable(const char * pTblName, const char * pFileName, int openMode, DbProvider * pDbP)
{
	Init(pDbP);
	open(pTblName, pFileName, openMode);
}
//
// Protected constructor
//
SLAPI DBTable::DBTable(const char * pTblName, const char * pFileName, void * pFlds, void * pData, int om, DbProvider * pDbP)
{
	class _DBField {
	public:
		int    hTbl;
		int    hFld;
	};
	Init(pDbP);
	RECORDSIZE s = 0;
	if(open(pTblName, pFileName, om)) {
		if(pFlds)
			for(int16 i = fields.getCount() - 1; i >= 0; i--) {
				((_DBField *)pFlds)[i].hTbl = handle;
				((_DBField *)pFlds)[i].hFld = i;
				s += (RECORDSIZE)stsize(fields[i].T);
			}
		if(pData)
			setDataBuf(pData, NZOR(s, fields.getRecSize()));
	}
}

SLAPI DBTable::~DBTable()
{
	close();
	if(State & sOwnDataBuf)
		ZFREE(buf);
	ZDELETE(P_Stmt);
	ZDELETE(P_OppStmt);
}

int DBTable::Debug_Output(SString & rBuf) const
{
	int    ok = 1;
	uint   i;
	rBuf = 0;
	CAT_FLD(handle, rBuf).CR();
	CAT_FLD(flags, rBuf).CR();
	CAT_FLD(tableID, rBuf).CR();
	CAT_FLD(ownrLvl, rBuf).CR();
	CAT_FLD(tableName, rBuf).CR();
	CAT_FLD(fileName, rBuf).CR();
	CAT_FLD(fields.getCount(), rBuf).CR();
	for(i = 0; i < fields.getCount(); i++) {
		CAT_FLD(fields.getField(i).Id, rBuf.Tab()).CR();
		CAT_FLD(fields.getField(i).Name, rBuf.Tab()).CR();
		CAT_FLD(fields.getField(i).Offs, rBuf.Tab()).CR();
		CAT_FLD_HEX(fields.getField(i).T, rBuf.Tab()).CR();
	}
	CAT_FLD(indexes.getNumKeys(), rBuf).CR();
	for(i = 0; i < indexes.getNumKeys(); i++) {
		CAT_FLD(indexes[i].getKeyNumber(), rBuf.Tab()).CR();
		CAT_FLD_HEX((long)indexes[i].getFlags(), rBuf.Tab()).CR();
		CAT_FLD(indexes[i].getNumSeg(), rBuf.Tab()).CR();
		for(int j = 0; j < indexes[i].getNumSeg(); j++) {
			CAT_FLD(indexes[i].getFieldID(j), rBuf.Tab(2)).CR();
			CAT_FLD_HEX((long)indexes[i].getFlags(j), rBuf.Tab(2)).CR();
		}
	}
	return ok;
}

int SLAPI DBTable::open(const char * pTblName, const char * pFileName, int openMode)
{
	SString temp_file_name;
	if(handle == 0) {
		char   p[64];
		if(pFileName == DBTable::CrTempFileNamePtr) {
			assert(pTblName != 0);
			pFileName = 0;
			THROW(P_Db && P_Db->CreateTempFile(pTblName, temp_file_name, 0));
			pFileName = temp_file_name;
			flags |= XTF_TEMP;
		}
		if(pTblName) {
			if(!tableID) {
				THROW(P_Db && P_Db->LoadTableSpec(this, pTblName, pFileName, 1));
			}
		}
		else {
			assert(pFileName != 0);
			fileName = pFileName;
		}
		DBS.GetProtectData(p, 1);
		if(P_Db) {
			THROW(P_Db->Implement_Open(this, fileName, openMode, p));
		}
		else {
			THROW(Btr_Open(fileName, openMode, p));
		}
		State |= sOpened_;
		memzero(p, sizeof(p));
		THROW(handle = DBS.GetTLA().AddTableEntry(this));
	}
	CATCH
		handle = 0;
		if(OpenExceptionProc) {
			if(fileName.Empty())
				fileName = NZOR(pFileName, pTblName);
			OpenExceptionProc(fileName, BtrError);
		}
	ENDCATCH
	return handle;
}

int SLAPI DBTable::close()
{
	if(State & sOpened_) {
		if(P_Db)
			P_Db->Implement_Close(this);
		else {
			Btr_Close();
		}
		State &= ~sOpened_;
	}
	if(handle) {
		tableName[0] = 0;
		fileName = 0;
		fields.reset();
		indexes.reset();
		DBS.GetTLA().FreeTableEntry(handle);
		handle = 0;
	}
	return 1;
}

int SLAPI DBTable::isOpen() const
{
	return (handle != 0);
}

int SLAPI DBTable::getField(uint fldN, DBField * pFld) const
{
	if(fldN < fields.getCount()) {
		DBField fld;
		fld.id = handle;
		fld.fld = fldN;
		ASSIGN_PTR(pFld, fld);
		return 1;
	}
	else
		return 0;
}

int SLAPI DBTable::getFieldByName(const char * pName, DBField * pFld) const
{
	uint   pos = 0;
	const  BNField * f = &fields.getField(pName, &pos);
	if(f) {
		DBField fld;
		fld.id = handle;
		fld.fld = pos;
		ASSIGN_PTR(pFld, fld);
		return 1;
	}
	else
		return 0;
}

int SLAPI DBTable::getFieldValue(uint fldN, void * pBuf, size_t * pSize) const
{
	return (buf && fldN < fields.getCount()) ? fields[fldN].getValue(buf, pBuf, pSize) : 0;
}

int SLAPI DBTable::setFieldValue(uint fldN, const void * pBuf)
{
	return (buf && fldN < fields.getCount()) ? fields[fldN].setValue(buf, pBuf) : 0;
}

int SLAPI DBTable::getFieldValByName(const char * pName, void * pVal, size_t * pSize) const
{
	const  BNField * f = &fields.getField(pName, 0);
	return (buf && f) ? f->getValue(buf, pVal, pSize) : 0;
}

int SLAPI DBTable::setFieldValByName(const char * pName, const void * pVal)
{
	const  BNField * f = &fields.getField(pName, 0);
	return (f && buf) ? f->setValue(buf, pVal) : 0;
}

int SLAPI DBTable::putRecToString(SString & rBuf, int withFieldNames)
{
	rBuf = 0;
	for(uint i = 0; i < fields.getCount(); i++) {
		char   temp_buf[1024];
		const BNField & f = fields[i];
		f.putValueToString(buf, temp_buf);
		if(withFieldNames)
			rBuf.CatEq(f.Name, temp_buf);
		else
			rBuf.Cat(temp_buf);
		rBuf.Semicol().Space();
	}
	return 1;
}

int DBTable::allocOwnBuffer(int size)
{
	int    ok = 1;
	RECORDSIZE rec_size = (size < 0) ? fields.getRecSize() : (RECORDSIZE)size;
	if(buf && State & sOwnDataBuf) {
		ZFREE(buf);
	}
	buf = (char *)SAlloc::C(rec_size+1, 1);
	if(buf)
		bufLen = rec_size;
	else {
		bufLen = 0;
		ok = 0;
	}
	return ok;
}

void FASTCALL DBTable::setDataBuf(void * pBuf, RECORDSIZE aBufLen)
{
	if(State & sOwnDataBuf)
		ZFREE(buf);
	buf    = pBuf;
	bufLen = aBufLen;
}

void FASTCALL DBTable::setBuffer(SBaseBuffer & rBuf)
{
	if(State & sOwnDataBuf)
		ZFREE(buf);
	buf    = rBuf.P_Buf;
	bufLen = (RECORDSIZE)rBuf.Size;
}

const SBaseBuffer FASTCALL DBTable::getBuffer() const
{
	SBaseBuffer ret_buf;
	ret_buf.P_Buf = (char *)buf; // @trick
	ret_buf.Size = bufLen;
	return ret_buf;
}

void SLAPI DBTable::clearDataBuf()
{
	memzero(buf, bufLen);
}

void FASTCALL DBTable::copyBufFrom(const void * pBuf)
{
	if(pBuf && buf) {
		memcpy(buf, pBuf, bufLen);
	}
}

void FASTCALL DBTable::copyBufFrom(const void * pBuf, size_t srcBufSize)
{
	if(pBuf && buf) {
		size_t s = (srcBufSize && srcBufSize < bufLen) ? srcBufSize : bufLen;
		memcpy(buf, pBuf, s);
	}
}

void FASTCALL DBTable::copyBufTo(void * pBuf) const
{
	if(pBuf && buf)
		memcpy(pBuf, buf, bufLen);
}

int SLAPI DBTable::copyBufToKey(int idx, void * pKey) const
{
	int    ok = 1;
	if(!pKey)
		ok = -1;
	else if(idx >= 0 && idx < (int)indexes.getNumKeys()) {
		BNKey k = indexes.getKey(idx);
		const int ns = k.getNumSeg();
		size_t offs = 0;
		for(int i = 0; i < ns; i++) {
			size_t sz;
			indexes.field(idx, i).getValue(buf, PTR8(pKey)+offs, &sz);
			offs += sz;
		}
	}
	else
		ok = 0; // @todo(errdef)
	return ok;
}

RECORDSIZE SLAPI DBTable::getBufLen() const
{
	return bufLen;
}

DBRowId * DBTable::getCurRowIdPtr()
{
	return &CurRowId;
}

int FASTCALL DBTable::HasNote(DBField * pLastFld) const
{
	int    ok = -1;
	if(State & sHasNote) {
		ok = 1;
		if(pLastFld) {
			int    last_note_field_found = getField(fields.getCount()-1, pLastFld);
			if(!last_note_field_found) {
				assert(last_note_field_found);
				ok = 0;
			}
			else {
				int    t_ = GETSTYPE(pLastFld->getField().T);
				if(t_ != S_NOTE) {
					assert(t_ == S_NOTE);
					ok = 0;
				}
			}
		}
	}
	return ok;
}

int FASTCALL DBTable::HasLob(DBField * pLastFld) const
{
	int    ok = -1;
	if(State & sHasLob) {
		ok = 1;
		if(pLastFld) {
			int    last_lob_field_found = getField(fields.getCount()-1, pLastFld);
			if(!last_lob_field_found) {
				assert(last_lob_field_found);
				ok = 0;
			}
			else {
				int    t_ = GETSTYPE(pLastFld->getField().T);
				if(!oneof2(t_, S_BLOB, S_CLOB)) {
					assert(oneof2(t_, S_BLOB, S_CLOB));
					ok = 0;
				}
			}
		}
	}
	return ok;
}

int SLAPI DBTable::InitLob()
{
	int    ok = 0;
	State &= ~(sHasLob | sHasNote);
	LobB.clear();
	for(uint i = 0; i < fields.getCount(); i++) {
		int _t = GETSTYPE(fields[i].T);
		if(oneof2(_t, S_BLOB, S_CLOB)) {
			DBLobItem item;
			item.FldN = i;
			item.Size = 0;
			item.Loc = 0;
			LobB.insert(&item);
			State |= sHasLob;
			ok = 1;
		}
		else if(_t == S_NOTE) {
			State |= sHasNote;
		}
		else if(_t == S_AUTOINC)
			State |= sHasAutoinc;
	}
	return ok;
	//SETFLAG(State, sHasLob, (LobB.Init(fields) > 0));
	//return BIN(State & sHasLob);
}

uint SLAPI DBTable::GetLobCount() const
{
	return LobB.getCount();
}

int SLAPI DBTable::GetLobField(uint n, DBField * pFld) const
{
	int    ok = 0;
	if(pFld) {
		pFld->id = 0;
		pFld->fld = 0;
	}
	if(n < LobB.getCount()) {
		uint fld_n = ((DBLobItem *)LobB.at(n))->FldN;
		if(fld_n < fields.getCount()) {
			if(pFld) {
				pFld->id = handle;
				pFld->fld = (int)fld_n;
			}
			ok = 1;
		}
	}
	return ok;
}

DBLobBlock * DBTable::getLobBlock()
{
	return &LobB;
}

int DBTable::setLobSize(DBField fld, size_t sz)
{
	return LobB.SetSize((uint)fld.fld, sz);
}

int DBTable::getLobSize(DBField fld, size_t * pSz) const
{
	return LobB.GetSize((uint)fld.fld, pSz);
}

int DBTable::readLobData(DBField fld, SBuffer & rBuf) const
{
	int    ok = -1;
	size_t sz;
	if(LobB.GetSize((uint)fld.fld, &sz) > 0) {
		if(sz) {
			SLob * p_fld_data = (SLob *)fld.getValuePtr();
			const void * ptr = p_fld_data->GetRawDataPtr();
			if(ptr) {
				rBuf.Write(ptr, sz);
				ok = 1;
			}
		}
	}
	return ok;
}

int DBTable::writeLobData(DBField fld, const void * pBuf, size_t dataSize, int forceCanonical /*=0*/)
{
	int    ok = -1, r;
	size_t sz;
	//
	// �������� �� ��, ��� ���� fld ������������� �������� LOB'��
	//
	THROW(r = LobB.GetSize((uint)fld.fld, &sz));
	if(r > 0) {
		size_t flat_size = fld.getField().size();
		SLob * p_fld_data = (SLob *)fld.getValuePtr();
		void * ptr = p_fld_data->GetRawDataPtr();
		STempBuffer temp_buf(0);
		if(ptr == pBuf && (dataSize > flat_size || forceCanonical)) {
			//
			// ���� �������� ��������� ����� ��������� �� ������ LOB, �� ����������
			// ����������� ���� �� ���������� ������ ����� ���, ��� ������� InitPtr
			// ����������� SLob � ��������� � �������������.
			//
			THROW(temp_buf.Alloc(dataSize));
			memcpy(temp_buf, pBuf, dataSize);
			pBuf = temp_buf;
		}
		THROW(p_fld_data->InitPtr((dataSize > flat_size || forceCanonical) ? dataSize : 0));
		THROW(ptr = p_fld_data->GetRawDataPtr());
		if(ptr != pBuf)
			memcpy(ptr, pBuf, dataSize);
		THROW(setLobSize(fld, dataSize));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI DBTable::StoreAndTrimLob()
{
	int    ok = -1;
	if(State & sHasLob) {
		LobB.Storage.Clear();
		for(uint i = 0; i < fields.getCount(); i++) {
			const BNField & r_fld = fields[i];
			if(oneof2(GETSTYPE(r_fld.T), S_BLOB, S_CLOB)) {
				uint   lob_pos = 0;
				THROW_DS(LobB.SearchPos(i, &lob_pos));
				DBLobItem * p_lob_item = (DBLobItem *)LobB.at(lob_pos);
				uint32 lob_sz = p_lob_item->Size;
				LobB.Storage.Write(lob_sz);
				if(lob_sz) {
					SLob * p_lob = (SLob *)(PTR8(buf)+r_fld.Offs);
					THROW_DS(p_lob->Serialize(+1, stsize(r_fld.T), &p_lob_item->StrgInd, LobB.Storage));
					p_lob->Empty();
					p_lob_item->Size = 0;
					ok = 2;
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI DBTable::RestoreLob()
{
	int    ok = -1;
	if(State & sHasLob) {
		assert(LobB.Storage.GetAvailableSize() != 0);
		for(uint i = 0; i < fields.getCount(); i++) {
			const BNField & r_fld = fields[i];
			if(oneof2(GETSTYPE(r_fld.T), S_BLOB, S_CLOB)) {
				uint   lob_pos = 0;
				THROW_DS(LobB.SearchPos(i, &lob_pos));
				DBLobItem * p_lob_item = (DBLobItem *)LobB.at(lob_pos);
				uint32 lob_sz = 0;
				THROW(LobB.Storage.Read(lob_sz));
				if(lob_sz) {
					SLob * p_lob = (SLob *)(PTR8(buf)+r_fld.Offs);
					THROW_DS(p_lob->Serialize(-1, stsize(r_fld.T), &p_lob_item->StrgInd, LobB.Storage));
				}
				p_lob_item->Size = lob_sz;
				ok = 1;
			}
		}
	}
	CATCHZOK
	LobB.Storage.Clear();
	return ok;
}

int FASTCALL DBTable::getRecSize(RECORDSIZE * pRecSize) const
{
	ASSIGN_PTR(pRecSize, FixRecSize);
	return 1;
}

int FASTCALL DBTable::insertRecBuf(const void * pData)
{
	OutOfTransactionLogging("insertRec");
	return P_Db ? P_Db->Implement_InsertRec(this, -1, 0, pData) : Btr_Implement_InsertRec(-1, 0, pData);
}

int SLAPI DBTable::insertRecBuf(const void * pData, int idx, void * pKeyBuf)
{
	OutOfTransactionLogging("insertRec");
	return P_Db ? P_Db->Implement_InsertRec(this, idx, pKeyBuf, pData) : Btr_Implement_InsertRec(idx, pKeyBuf, pData);
}

int FASTCALL DBTable::insertRec()
{
	OutOfTransactionLogging("insertRec");
	if(P_Db)
		return P_Db->Implement_InsertRec(this, -1, 0, 0);
	else
		return Btr_Implement_InsertRec(-1, 0, 0);
}

int FASTCALL DBTable::insertRec(int idx, void * pKeyBuf)
{
	OutOfTransactionLogging("insertRec");
	return P_Db ? P_Db->Implement_InsertRec(this, idx, pKeyBuf, 0) : Btr_Implement_InsertRec(idx, pKeyBuf, 0);
}

void FASTCALL DBTable::OutOfTransactionLogging(const char * pOp) const
{
	if(!(flags & (XTF_TEMP|XTF_DICT|XTF_DISABLEOUTOFTAMSG)) && !(DBS.GetTLA().GetState() & DbThreadLocalArea::stTransaction)) {
		SString msg_buf;
		msg_buf.Cat(getcurdatetime_()).CatChar('\t').Cat(pOp).Space().Cat("executed out of transaction").CatDiv(':', 2).
			Cat(tableName).CatChar('(').Cat(fileName).CatChar(')');
		SLS.LogMessage("dbwarn.log", msg_buf);
	}
}

int FASTCALL DBTable::updateRec()
{
	OutOfTransactionLogging("updateRec");
	return P_Db ? P_Db->Implement_UpdateRec(this, 0, 0) : Btr_Implement_UpdateRec(0, 0);
}

int FASTCALL DBTable::updateRecNCC()
{
	OutOfTransactionLogging("updateRec");
	return P_Db ? P_Db->Implement_UpdateRec(this, 0, 1) : Btr_Implement_UpdateRec(0, 1);
}

int FASTCALL DBTable::updateRecBuf(const void * pDataBuf)
{
	OutOfTransactionLogging("updateRec");
	return P_Db ? P_Db->Implement_UpdateRec(this, pDataBuf, 0) : Btr_Implement_UpdateRec(pDataBuf, 0);
}

int SLAPI DBTable::deleteRec()
{
	OutOfTransactionLogging("deleteRec");
	return P_Db ? P_Db->Implement_DeleteRec(this) : Btr_Implement_DeleteRec();
}

int SLAPI DBTable::deleteByQuery(int useTa, DBQ & rQ)
{
	int    ok = 1;
	if(P_Db) {
		ok = P_Db->Implement_DeleteFrom(this, useTa, rQ);
	}
	else {
		DBQuery * q = & selectAll().from(this, 0L).where(rQ);
		q->setDestroyTablesMode(0);
		// @v8.1.4 q->setSearchForUpdateMode(1);
		if(!useTa || Btrieve::StartTransaction(1)) {
			for(int dir = spFirst; ok && q->single_fetch(0, 0, dir); dir = spNext) {
				// @v8.1.4 {
				uint8  key_buf[512];
				DBRowId _dbpos;
				if(!getPosition(&_dbpos))
					ok = 0;
				else if(!getDirectForUpdate(getCurIndex(), key_buf, _dbpos))
					ok = 0;
				else { // } @v8.1.4
					if(deleteRec() == 0) // @sfu
						ok = 0;
				}
			}
			if(q->error)
				ok = 0;
			if(useTa)
				if(ok) {
					if(!Btrieve::CommitWork()) {
						Btrieve::RollbackWork();
						ok = 0;
					}
				}
				else
					Btrieve::RollbackWork();
		}
		else
			ok = 0;
		delete q;
	}
	return ok;
}

int FASTCALL DBTable::getPosition(DBRowId * pPos)
{
	return P_Db ? P_Db->Implement_GetPosition(this, pPos) : Btr_Implement_GetPosition(pPos);
}

int SLAPI DBTable::getDirect(int idx, void * pKey, const DBRowId & rPos)
{
	char   k[MAXKEYLEN];
	memcpy(k, &rPos, sizeof(rPos));
#define sf (sfDirect)
	int    ok = P_Db ? P_Db->Implement_Search(this, idx, k, 0, sf) : Btr_Implement_Search(idx, k, 0, sf);
#undef sf
	if(pKey && ok)
		memcpy(pKey, k, indexes.getKeySize((idx >= 0) ? idx : index));
	return ok;
}

int SLAPI DBTable::rereadForUpdate(int idx, void * pKey)
{
	int    ok = 1;
	DbSession::Config dbcfg;
	DBS.GetConfig(dbcfg);
	if(dbcfg.NWaitLockTries != BTR_RECLOCKDISABLE) { // @v8.6.5 ��� ������������� ������������ ������, ���� ���������� �� �����������
		uint8  _key[512];
		if(!pKey) {
			MEMSZERO(_key);
			pKey = _key;
		}
		if(idx < 0)
			idx = 0;
		DBRowId _dbpos;
		THROW(getPosition(&_dbpos));
		THROW(getDirectForUpdate(idx, pKey, _dbpos));
	}
	CATCHZOK
	return ok;
}

int SLAPI DBTable::getDirectForUpdate(int idx, void * pKey, const DBRowId & rPos)
{
	char   k[MAXKEYLEN];
	memcpy(k, &rPos, sizeof(rPos));
#define sf (sfDirect|sfForUpdate)
	int    ok = P_Db ? P_Db->Implement_Search(this, idx, k, 0, sf) : Btr_Implement_Search(idx, k, 0, sf);
#undef sf
	if(pKey && ok)
		memcpy(pKey, k, indexes.getKeySize((idx >= 0) ? idx : index));
	return ok;
}

int FASTCALL DBTable::searchForUpdate(void * pKey, int srchMode)
{
	return P_Db ? P_Db->Implement_Search(this, index, pKey, srchMode, sfForUpdate) : Btr_Implement_Search(index, pKey, srchMode, sfForUpdate);
}

int FASTCALL DBTable::searchForUpdate(int idx, void * pKey, int srchMode)
{
	return P_Db ? P_Db->Implement_Search(this, idx, pKey, srchMode, sfForUpdate) : Btr_Implement_Search(idx, pKey, srchMode, sfForUpdate);
}

int FASTCALL DBTable::search(int idx, void * pKey, int srchMode)
{
	return P_Db ? P_Db->Implement_Search(this, idx, pKey, srchMode, 0) : Btr_Implement_Search(idx, pKey, srchMode, 0);
}

int FASTCALL DBTable::search(void * pKey, int srchMode)
{
	return P_Db ? P_Db->Implement_Search(this, index, pKey, srchMode, 0) : Btr_Implement_Search(index, pKey, srchMode, 0);
}

int SLAPI DBTable::searchKey(int idx, void * pKey, int srchMode)
{
	return P_Db ? P_Db->Implement_Search(this, idx, pKey, srchMode, sfKeyOnly) : Btr_Implement_Search(idx, pKey, srchMode, sfKeyOnly);
}

int SLAPI DBTable::GetFileStat(long reqItems, DbTableStat * pStat)
{
	return P_Db ? P_Db->GetFileStat(this, reqItems, pStat) : Btr_GetStat(reqItems, pStat);
}

int SLAPI DBTable::SerializeSpec(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = -1;
	SString tbl_name;
	if(dir > 0)
		tbl_name = tableName;
	THROW(pCtx->Serialize(dir, tbl_name, rBuf));
	if(dir < 0)
		tbl_name.CopyTo(tableName, sizeof(tableName));
	THROW(pCtx->Serialize(dir, fileName, rBuf));
	THROW(pCtx->SerializeFieldList(dir, &fields, rBuf));
	THROW(indexes.Serialize(dir, rBuf, pCtx));
	CATCHZOK
	return ok;
}

int SLAPI DBTable::SerializeRecord(int dir, void * pRec, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = -1;
	if(dir > 0) {
		ok = pCtx->Serialize(tableName, &fields, pRec, rBuf);
	}
	else if(dir < 0) {
		ok = pCtx->Unserialize(&fields, pRec, rBuf);
	}
	return ok;
}

int SLAPI DBTable::SerializeArrayOfRecords(int dir, SArray * pList, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	int32  c = pList ? pList->getCount() : 0; // @persistent
	STempBuffer temp_buf(0);
	if(dir < 0 && pList)
		pList->freeAll();
	THROW(pCtx->Serialize(dir, c, rBuf));
	for(int32 i = 0; i < c; i++) {
		if(pList) {
			THROW(temp_buf.Alloc(pList->getItemSize()+4096)); // +4096 ���������
		}
		else {
			THROW(temp_buf.Alloc(8192));
		}
		if(dir > 0) {
			memcpy(temp_buf, pList->at(i), pList->getItemSize());
		}
		THROW(SerializeRecord(dir, temp_buf, rBuf, pCtx));
		if(dir < 0 && pList) {
			THROW(pList->insert(temp_buf));
		}
	}
	CATCHZOK
	return ok;
}
