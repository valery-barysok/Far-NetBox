//---------------------------------------------------------------------------
#include "stdafx.h"

#include "Common.h"
#include "WinSCPSecurity.h"
//---------------------------------------------------------------------------
#define PWALG_SIMPLE_INTERNAL 0x00
#define PWALG_SIMPLE_EXTERNAL 0x01
//---------------------------------------------------------------------------
int random(int range)
{
  return static_cast<int>(static_cast<double>(rand()) / (static_cast<double>(RAND_MAX) / range));
}

//---------------------------------------------------------------------------
std::string SimpleEncryptChar(unsigned char Ch)
{
  Ch = (unsigned char)((~Ch) ^ PWALG_SIMPLE_MAGIC);
  return
    PWALG_SIMPLE_STRING.SubString(((Ch & 0xF0) >> 4), 1) +
    PWALG_SIMPLE_STRING.SubString(((Ch & 0x0F) >> 0), 1);
}
//---------------------------------------------------------------------------
unsigned char SimpleDecryptNextChar(std::string & Str)
{
  if (Str.Length() > 0)
  {
    unsigned char Result = (unsigned char)
                           ~((((PWALG_SIMPLE_STRING.find_first_of(Str.c_str()[0])) << 4) +
                              ((PWALG_SIMPLE_STRING.find_first_of(Str.c_str()[1])) << 0)) ^ PWALG_SIMPLE_MAGIC);
    Str.Delete(0, 2);
    return Result;
  }
  else { return 0x00; }
}
//---------------------------------------------------------------------------
UnicodeString EncryptPassword(const UnicodeString Password, const UnicodeString Key, int /* Algorithm */)
{
  std::string Result("");
  size_t Shift = 0;
  size_t Index = 0;

  // if (!RandSeed) Randomize();
  std::string Password2 = W2MB((Key + Password).c_str());
  Shift = (Password2.Length() < PWALG_SIMPLE_MAXLEN) ?
          static_cast<unsigned char>(random(PWALG_SIMPLE_MAXLEN - Password2.Length())) : 0;
  // DEBUG_PRINTF(L"Shift = %d", Shift);
  Result += SimpleEncryptChar(static_cast<unsigned char>(PWALG_SIMPLE_FLAG)); // Flag
  Result += SimpleEncryptChar(static_cast<unsigned char>(PWALG_SIMPLE_INTERNAL)); // Dummy
  Result += SimpleEncryptChar(static_cast<unsigned char>(Password2.Length()));
  Result += SimpleEncryptChar(static_cast<unsigned char>(Shift));
  for (Index = 0; Index < Shift; Index++)
  {
    Result += SimpleEncryptChar(static_cast<unsigned char>(random(256)));
  }
  for (Index = 0; Index < Password2.Length(); Index++)
  {
    Result += SimpleEncryptChar(Password2.c_str()[Index]);
  }
  while (Result.Length() < PWALG_SIMPLE_MAXLEN * 2)
  {
    Result += SimpleEncryptChar(static_cast<unsigned char>(random(256)));
  }
  return MB2W(Result.c_str());
}
//---------------------------------------------------------------------------
UnicodeString DecryptPassword(const UnicodeString Password, const UnicodeString Key, int /* Algorithm */)
{
  std::string Result("");
  int Index;
  unsigned char Length, Flag;
  std::string Password2 = W2MB(Password.c_str());
  std::string Key2 = W2MB(Key.c_str());
  Flag = SimpleDecryptNextChar(Password2);
  // DEBUG_PRINTF(L"Flag = %x, PWALG_SIMPLE_FLAG = %x", Flag, PWALG_SIMPLE_FLAG);
  if (Flag == (unsigned char)PWALG_SIMPLE_FLAG)
  {
    /* Dummy = */ SimpleDecryptNextChar(Password2);
    Length = SimpleDecryptNextChar(Password2);
    // DEBUG_PRINTF(L"Length = %d", Length);
  }
  else { Length = PWALG_SIMPLE_FLAG; }
  // DEBUG_PRINTF(L"Length = %d", Length);
  Password2.Delete(0, (static_cast<int>(SimpleDecryptNextChar(Password2))*2));
  for (Index = 0; Index < Length; Index++)
  {
    Result += static_cast<char>(SimpleDecryptNextChar(Password2));
  }
  if (Flag == PWALG_SIMPLE_FLAG)
  {
    if (Result.SubString(0, Key.Length()) != Key2) { Result = ""; }
    else { Result.Delete(0, Key2.Length()); }
  }
  return MB2W(Result.c_str());
}
//---------------------------------------------------------------------------
UnicodeString SetExternalEncryptedPassword(const UnicodeString Password)
{
  std::string Result;
  Result += SimpleEncryptChar(static_cast<unsigned char>(PWALG_SIMPLE_FLAG));
  Result += SimpleEncryptChar(static_cast<unsigned char>(PWALG_SIMPLE_EXTERNAL));
  Result += W2MB(StrToHex(Password).c_str());
  return MB2W(Result.c_str());
}
//---------------------------------------------------------------------------
bool GetExternalEncryptedPassword(const UnicodeString Encrypted, UnicodeString & Password)
{
  std::string Encrypted2 = W2MB(Encrypted.c_str());
  bool Result =
    (SimpleDecryptNextChar(Encrypted2) == PWALG_SIMPLE_FLAG) &&
    (SimpleDecryptNextChar(Encrypted2) == PWALG_SIMPLE_EXTERNAL);
  if (Result)
  {
    Password = ::HexToStr(MB2W(Encrypted2.c_str()));
  }
  return Result;
}
//---------------------------------------------------------------------------
