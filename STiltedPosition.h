/*
Class Definition of the Position Class
Contains static position attributes,
PSQT evaluation, make/unmake move, zobrist hashing

2nd Refactoring

7-15-2024, TheTilted096

*/

#include "STiltedBitboards.h"

class Position : public Bitboards{
    protected:
        Move moves[96][128];
        int mprior[96][128];

        int scores[2], eScores[2];
        //int mobil[2], eMobil[2];

        int thm, chm[1024];
        Hash zhist[1024];

        int gamePhase;
        
        int historyTable[2][6][64];

        //tunables

        static constexpr uint8_t totalGamePhase = 64;
        static constexpr uint8_t phases[6] = {0, 6, 4, 2, 1, 1};
        static constexpr int matVals[6] = {0, 650, 400, 200, 150, 100};

        static constexpr Hash zpk[2][6][64] =
        {{{0x2FA823ED9B2821E4ULL, 0xA225E44E696101BBULL, 0xB22550AFD86BEF49ULL, 0xB19975CB60376CULL, 
        0xA5E0EF81276029D6ULL, 0xEFE12405B4E03D26ULL, 0x1AA63ADDB0E2F891ULL, 0x5E348C958E35025ULL, 
        0x9E519BE627EF2B48ULL, 0x6637DE4BAE42F56FULL, 0x870ADCDD93DB99D5ULL, 0xA7C6627A14E7BE00ULL, 
        0x17B6DD854304F333ULL, 0x75DB90B6EC14140AULL, 0x4EFAFBF195E365E7ULL, 0xAF2EE39B09D90344ULL, 
        0x80EE7EFD1BCAFC17ULL, 0x4603B872058173B7ULL, 0x47FC8E1DBB9E8908ULL, 0x6FB93A846AD083ACULL, 
        0x7C0831F9944D43C6ULL, 0x6254B6EF4078C4BULL, 0xB434F6873F2917A5ULL, 0xB9F486B0B303101DULL, 
        0xAC574A08E302E0CBULL, 0x1D0466A02A2FF563ULL, 0xA8A1E9EE165362A8ULL, 0x6BA9F471C732B075ULL, 
        0xF5D1DCD1F530D093ULL, 0xE78921C04EEB3CD8ULL, 0x57CD49517CEF01B0ULL, 0xB046C7AFE0F85637ULL, 
        0xF59CF6F885F4DECDULL, 0x5B6D1D9721681BC8ULL, 0x2DA8E5D3289059FBULL, 0x217D8BF34E617D5DULL, 
        0xE31B0ADA6ADF1F3DULL, 0xE8E79472473C3921ULL, 0x3C41CF4CF7EC0B0CULL, 0x77B437D7B45D2B0BULL, 
        0xA4A390B063DF6DD4ULL, 0x909DF069757D23C8ULL, 0xE3046DB836D9314BULL, 0x9F21DB67348B0A25ULL, 
        0xA214FF847D11E4FEULL, 0xE422732C52BC6A68ULL, 0x7A99BA70000080CBULL, 0x45A9F8264473ACA4ULL, 
        0xEEFC293BF528340BULL, 0x590560C5AF74F48AULL, 0x7631FAF80C177E6AULL, 0x675421CE3B8C940AULL, 
        0xFB55C03ADFFAB17AULL, 0x405CBF1DCAD09BA6ULL, 0x85EE851A21A002F5ULL, 0x2972BD8F986C5A53ULL, 
        0xF8EDB6F56F97294EULL, 0xF4E95B783D03B0CCULL, 0x1BFBD68916D1C823ULL, 0x34161DE7C308D48CULL, 
        0x6CA11B9FEE0349EDULL, 0x36E2A58FA14EAFF1ULL, 0xB86EE421A99D1EE0ULL, 0xB109D614B6DC8590ULL}, {
        0x83F954E2B265BD90ULL, 0x2AC8B477C650524DULL, 0x731C914A561F040DULL, 0x4E253620AE87535AULL, 
        0x3AEB1AF36B31A25CULL, 0x8AA24C43A667490AULL, 0xBD977D175B6FD5A7ULL, 0x8B946F43EE89052FULL, 
        0x163F70A38A5E7227ULL, 0x187DFBABA4E9FD38ULL, 0x722458F226CE18E1ULL, 0xF3F27816FD68F4AAULL, 
        0x4410DF64E600446CULL, 0x59048A4641698830ULL, 0x908E2467F6D1550CULL, 0xE786CC4952620673ULL, 
        0x4196C722791AAB9EULL, 0xA43D33368D793FAEULL, 0x5A41DE2870DE9FCCULL, 0x715E8B62D9474AFULL, 
        0xF682D17F8BE46DAFULL, 0x86436F1C8F2E3D0EULL, 0xED1418D93FBFA20CULL, 0xC82DD5751E0A7B2AULL, 
        0x523F3A9ADB12B976ULL, 0x4B18F6702611C70AULL, 0xDCBEEF69CCDFDD3BULL, 0x25A94FF5F0875F73ULL, 
        0xE530DA958DF828C2ULL, 0xF79FAF4F133AAD5EULL, 0xE20B1EECCB026E04ULL, 0x80E43C8513FEF546ULL, 
        0x52D75208229AA413ULL, 0x187B98B02DAFC93EULL, 0xA0B224F7E7E87FC2ULL, 0xB7188A4B974F1197ULL, 
        0x7D87983B9BE15F5AULL, 0x9DE2F96A196C15F6ULL, 0x23B1CA9574E29E31ULL, 0xF14FD1EB212E25CCULL, 
        0xCF9196ED2D403A20ULL, 0x29A3FDC82F3C82CFULL, 0xC55912987C1529EEULL, 0x7BD303C260507495ULL, 
        0x90104BF6E4E77C1DULL, 0xC668A1C706458EC2ULL, 0x15A9C55D9DDE328EULL, 0x71B34EE3DD596914ULL, 
        0xC49D2EA0D6B93FDDULL, 0xF778A4CF602A742AULL, 0xD897E6206F01AB1BULL, 0xA70166F5089E2D16ULL, 
        0x82ED15462ACA0534ULL, 0xF309577B0545E6A7ULL, 0x2FEF38CC42686A17ULL, 0x15BFFC47B4E5E376ULL, 
        0xEE0545A1835AC85CULL, 0xDD0CA4F1F1DB95FULL, 0x4267C98495BFDC03ULL, 0x53E3E4ECF9053EC1ULL, 
        0xD0E3E9AFF05F01CEULL, 0x4C9D92D2E0CA043DULL, 0xEB7B52557CD62036ULL, 0x719BFBA1237824ACULL}, {
        0xBB1902D7BE99404DULL, 0xE51DD6C5F8F8B447ULL, 0xBFFA880F0A2E1A17ULL, 0xBEA021DC16231485ULL, 
        0xB60E40FA63BA9E06ULL, 0x2A1A37E56680A909ULL, 0xF88D279737E0DEA4ULL, 0x4B4D89F36D3FB7E2ULL, 
        0x50BAF9BA1013360AULL, 0x4B25C07F907C042BULL, 0x3867271488E12A1EULL, 0x4A1616398B769BEBULL, 
        0x83A3D7EF03852857ULL, 0x2EFE203C487A2EC2ULL, 0x7787CB58E2BD4D71ULL, 0xA4330D14476CA25DULL, 
        0x8BC80B58F7C4AF25ULL, 0x2D091475E79C286ULL, 0x7675AFE2964639FULL, 0x485DB4484DA0CB4CULL, 
        0x267D24D36844384AULL, 0xDE719669E725694ULL, 0xCB7777E4A2845E1ULL, 0x8E686A8737706669ULL, 
        0xF5C40EB30FB4BF23ULL, 0x5CC2E07D3E0B3772ULL, 0xB965FC4E3E54C8C3ULL, 0xD910517F4CD6EE5FULL, 
        0xFCF408837C842E7CULL, 0x3B8324097F3366FEULL, 0x455A6936D7FD2E65ULL, 0x36A2C5F2E3D1D3F9ULL, 
        0x9B40B31FE38E7913ULL, 0xC6F54A990B2E3E59ULL, 0x7FCC5746B1679D86ULL, 0xFBCA48D817159A10ULL, 
        0xD4A0E6F04AA64ED8ULL, 0xD489215D78F585D5ULL, 0xC211D3559A0A446BULL, 0x17DC3052211BB348ULL, 
        0x64B6C98D9E7BB717ULL, 0xA96CF3B19ED85C61ULL, 0xDA53F05169EE166CULL, 0x227AFC642D2A9DA2ULL, 
        0xC4D84ADB2C358746ULL, 0x149DFBE4FE6E5DF6ULL, 0xE8FD0578FFFE2A3BULL, 0xAB603A4113E633DDULL, 
        0xF1BDBE6AA5EC337BULL, 0x57A62B5FEA5BA4D0ULL, 0xF09E9CB1F9DDB61ULL, 0xA605A21433732364ULL, 
        0xC382B1DAF9D8737DULL, 0xB5438A8048E1622AULL, 0xDFD8AEE1CD39D39FULL, 0xF2A1255432DAA229ULL, 
        0x4179205706978F97ULL, 0x42EC262BD199A98AULL, 0x68988FAF27472103ULL, 0xD77722D55D6D445ULL, 
        0xD4E4F92F96953CBDULL, 0x5FEF615A2BF14A10ULL, 0x3623FC73D974ED34ULL, 0x79FDC4AFE688D9BCULL}, {
        0xC231A89FBF1DC708ULL, 0xC7B1EE085F423A23ULL, 0x52A97A86A994C7BBULL, 0xA91B055B2826EE6ULL, 
        0xFDD62ED95955DB2AULL, 0x39FC2F57BFF64014ULL, 0x678AAA0C42A6F86AULL, 0xFD70BA35458589D1ULL, 
        0xD9A5B10429463253ULL, 0xC09E55181E09C3A3ULL, 0x630130AABA9D1CB5ULL, 0x176AD4E43EB4C344ULL, 
        0x80D6571E38C275BEULL, 0xE349678CE223DFACULL, 0xD5DF4C22C5640F1FULL, 0x6CD006C7934C0D41ULL, 
        0x45249C1D63A0AE14ULL, 0xF3159B71B485C8CFULL, 0xA6A388DCC022A51CULL, 0xE82F2204583E4439ULL, 
        0x3520B48E8A9BB67EULL, 0xF4DB297CD5D1551FULL, 0x84E65FD0FA054A58ULL, 0x900AECC40C7C943CULL, 
        0x9F275C14F1EFF967ULL, 0xF08177C93D61C104ULL, 0x5D7C0BB7B44D94EBULL, 0xD9251F6B59409EB6ULL, 
        0xC068BD13E333BB74ULL, 0xEE2AD6BFBFFABC72ULL, 0xA6E7E9F8974EF2A9ULL, 0xE27D1537715BE2CCULL, 
        0x551B0DBBF3A56C99ULL, 0xD1E8B4F8746307D9ULL, 0xFA250FBC4C8AF6C3ULL, 0xFFBD1ADFE5F30E2CULL, 
        0xDB15AF7103E826CFULL, 0x5428794158FAA4AAULL, 0x8E0933B154A3D275ULL, 0xD6F4FF6E76D55D3BULL, 
        0x3E197E750D16454ULL, 0xE5098E8349F37FCFULL, 0x5FD000B23CAA428FULL, 0xBAFA175D71912790ULL, 
        0xE681879DCC191664ULL, 0xFA03C8DA7CBAC27BULL, 0xBD7CC2DAF82BB2DCULL, 0x539897D0C509F30EULL, 
        0x9993DAEFB43A940DULL, 0xD30C92D2712C7208ULL, 0xB4E065827815AD8DULL, 0x9878629C89E2E70EULL, 
        0xACD633CC8D72F09ULL, 0x907BA78660251D6EULL, 0x24443BD5C5CD946CULL, 0xB503A32C7530F297ULL, 
        0x9F7BD4AD1B8FE2B6ULL, 0x883087753D44B73CULL, 0x287560D48A5DAB6EULL, 0x440A6F4AC4D7648FULL, 
        0x8AB0EF67DA1397E0ULL, 0x967EF77780172F01ULL, 0x571C14D5A20C83A0ULL, 0x620A8883E63EEF90ULL}, {
        0xE12ADCC24DECB83AULL, 0x33DD9C669D6D1F8CULL, 0xB89B0914055769EULL, 0x9345BB0FD138677ULL, 
        0xC1A1521D2A73A461ULL, 0x88F6046BE586C58ULL, 0xB65960BE0FC460EAULL, 0x34D66980DE7DB88DULL, 
        0x3587634322D11249ULL, 0x3AA920C3BBBF60D7ULL, 0x237715F343EE5A41ULL, 0x588CEBCF4E3FC18FULL, 
        0x5EF1F98481A42DCDULL, 0x5031F50E35E37B3ULL, 0x6BACA87889479147ULL, 0x7CC4CA9C928B31A5ULL, 
        0x5B104FC8F7B2776AULL, 0xA259A51F5245D6A1ULL, 0x1DA61C2008C61322ULL, 0xE14CFB2819ADFE1ULL, 
        0x94C0CC08EE1E2CA2ULL, 0x6B66BFAFCF56A848ULL, 0x5F4CCFD2755BF751ULL, 0x137CABF49DC7F448ULL, 
        0x67EF7FA53D45004ULL, 0xD92465631C03F683ULL, 0xFE78603685E378B1ULL, 0x4F2032A4739703AULL, 
        0x26FD503E0E381080ULL, 0x9D517EBC356AB4D4ULL, 0x19094B0A60F247A9ULL, 0xDBDF326B920D1533ULL, 
        0x13A44233124BD3A3ULL, 0xE4D895ECD25A3FBAULL, 0xEEE98263CDE4CB56ULL, 0x324175C7D6B8B6F8ULL, 
        0xC989E662A8C727C6ULL, 0xA6F0C33F93D6F225ULL, 0x7C5D48203B2950C7ULL, 0x5C412864F16D119CULL, 
        0xDD94DA199B1DB568ULL, 0xD3753A07BB777BE8ULL, 0x2797DDE1D8698EFFULL, 0x72AE7D4A9860D50CULL, 
        0xBAA86D45B5D2B227ULL, 0x4388C46F1828ECD6ULL, 0x67C565C878251249ULL, 0x7C4F965EEDB547D1ULL, 
        0x9D5560803DDE189DULL, 0x738EE9FE03E548ECULL, 0x9EE8051867B2619CULL, 0x6D411C726C5A1084ULL, 
        0xCDEE42BDA258B73AULL, 0x72A1777E25D6BFB3ULL, 0x9E25F296168AA44EULL, 0xB99A2FA058963315ULL, 
        0x989A08286CC2094DULL, 0x17094D0DA34C724DULL, 0x8CEB9F4A447AD53FULL, 0xB332378551735AFFULL, 
        0x7A6AF9F710DA17FBULL, 0x32C8D0811BC0CA9ULL, 0xE517417127B8B2B9ULL, 0x835C5F1386B850F4ULL}, {
        0xEA65AD98289963F5ULL, 0x77F426B67829BDC6ULL, 0x469293D9AD8F1352ULL, 0x11232FDBC98AAC2EULL, 
        0xF96454E13677A94EULL, 0x9C7EB68D13C89F5EULL, 0x8217091702A89CB4ULL, 0xAD17ED0D9B5A81DEULL, 
        0xD3CF32531A4DF560ULL, 0x81C21B9BCE2745EFULL, 0x2801E30EAC116304ULL, 0xC8DE51E3606F5890ULL, 
        0x10DE6BE62447132DULL, 0xA919497570C7480DULL, 0x19AB75144425A186ULL, 0xF846F4B20D8A574EULL, 
        0xE64581C3E6E589D3ULL, 0x7137347D3DF0E1ACULL, 0xD2EFDC4189B482F0ULL, 0x83CEC3C7364B540DULL, 
        0x936ADEB04B1D69ACULL, 0x8FAE52BB382A3BCFULL, 0x330D26D903907611ULL, 0x9DDE84F50CD3F22AULL, 
        0x9CF7CF4946853E7ULL, 0x11965BBDCA16DB16ULL, 0x1F5B95C07FA22405ULL, 0x2B9AF5DFF7A76903ULL, 
        0x5B83F9D43602B272ULL, 0x95DB87F2BDE09A0FULL, 0xC35C71858360ED25ULL, 0xF64760CFC6C49F00ULL, 
        0xFE66A0E3CA3670A9ULL, 0x5DE6932E4EC9240ULL, 0x3FA4B7DE15E76474ULL, 0x4A268ACEE259B3DFULL, 
        0xECBF2E730AE38C91ULL, 0xFA19F9ACDC4AEB3ULL, 0x99C884FDC4F09FFAULL, 0xB2C23B18878B2947ULL, 
        0x574A5554FF0DC2E5ULL, 0x87837C22A9933B38ULL, 0xF7761C27D7E84179ULL, 0x8FB543A75501EC7ULL, 
        0x7AB9A20C4805E7C9ULL, 0x99192E043AD9245EULL, 0x2DC27432F3CB59F8ULL, 0x7E9C1812584714CULL, 
        0x6C07C24A707F7EDAULL, 0xC8165FBA3D2CE728ULL, 0x1BDEDE018FA9AF76ULL, 0x97C643B11555A332ULL, 
        0x9846E3D00912ED7ULL, 0xA5CF9148C5F74781ULL, 0x6098C321E8E4D4F9ULL, 0xCDF56516E7211959ULL, 
        0x5FEB934D5185FD93ULL, 0x7BCAB428F5A3BD5BULL, 0x81BD1390D6BC6DAFULL, 0xF93E4D856690654FULL, 
        0x72F5CA1486C33412ULL, 0x672AE10ECA9A03BDULL, 0xDC4ED1FD8EB661C0ULL, 0x10EB3839B3B786E2ULL}}, {{
        0x361BF30FD28E41A6ULL, 0x59AA12332AC35A2DULL, 0x64C648504301CE96ULL, 0xC8BC93143D58E078ULL, 
        0x416C6ECE25F513DBULL, 0xE60442CF0F53EF3EULL, 0xD5E70F6B4CE4FD24ULL, 0x66F85E07CC28AE8ULL, 
        0x2EFCBB2C7EA07FCDULL, 0x678C756D7506FC74ULL, 0xC8FF60F5BACBEBBDULL, 0xA15E4349201F59C4ULL, 
        0xCA10CE7F2CC2FBFBULL, 0xF69B2C8829C25029ULL, 0x2B89271AC1BD231FULL, 0xC8894F58B14EC00BULL, 
        0x15353E6377C338A0ULL, 0xEEAE5B20C9BC6217ULL, 0x2CA58E9EC3BED263ULL, 0xFD20ABC54ACAEDC1ULL, 
        0x18F59A2A46C474C4ULL, 0x3372DE1ED5E074CAULL, 0x32A91A776DEA195BULL, 0x3F94B89B239E548ULL, 
        0x20788E8FEF53A4FDULL, 0x875D4D2E96FF017ULL, 0xD2173264AA2C3FAFULL, 0x82DC16D63F39D962ULL, 
        0xF690C7E8A0763594ULL, 0x6CC1142540335DD6ULL, 0x15D0811BE57DF743ULL, 0xEFB2469AB642C71FULL, 
        0xB943039D7269D88FULL, 0xE00B34BC955E0151ULL, 0x65FD901EE8189D2BULL, 0x4DF6C3E1DEFB0ULL, 
        0x69082285335B289ULL, 0xAFDB6783AD7094B3ULL, 0x4E57156ECA335A5BULL, 0xE1FD950F62871F02ULL, 
        0xEC425A718C244E8FULL, 0x2C0B7D97CCB90955ULL, 0x6133F9CC23F4F3D6ULL, 0x7805E646627D8074ULL, 
        0x258D55BB7802A742ULL, 0x6D6FD82BEA618723ULL, 0x1D7E3A26062DE42FULL, 0x8244353B6DC5EF2AULL, 
        0x2FF9C8B37B8A598CULL, 0x94A63404042B2F56ULL, 0x57F0D6703E847C95ULL, 0xB85B80F7742FB7BAULL, 
        0x784136CB762FE334ULL, 0x6D0FEE2BD582B9AFULL, 0x522621A58430D1A9ULL, 0x2A3A85CE86768132ULL, 
        0xB429DB372C7A43E5ULL, 0xA9B7D5AFD72C1934ULL, 0xEB88DC054BBE0020ULL, 0x303CF9D60D5B9ULL, 
        0xABEF63DEE491D6FFULL, 0x105A5BACE96EC56DULL, 0xFCCD68677A47EAD0ULL, 0xD9CE26202F6296F1ULL}, {
        0xD59B5818A9A878CULL, 0xAD5A73FE009D1E49ULL, 0x9C7BD1BF3EAE3F4CULL, 0x524677991B8A6EE7ULL, 
        0x4CB3C710636F318EULL, 0x737FD84214E744DEULL, 0xA970FDF1F6C910D0ULL, 0xB55003011F62354CULL, 
        0x6B349C634FECE58BULL, 0x5B4B7B4DBEBF456EULL, 0xE66EB388D6DE1A10ULL, 0xCB6A194D33EFD2ECULL, 
        0x576AD4C2060B6F09ULL, 0xB1FE9CF714E1C563ULL, 0xC03E9AA2590DA044ULL, 0xCC3E2C21A2C4930FULL, 
        0xA24632ADD44FCA8BULL, 0xE25EC8EFC61D33A5ULL, 0x56A46BE1BE7747ECULL, 0xF15A1E52FEFF5ED4ULL, 
        0xE7496CA496CDA18CULL, 0x5A87AC40F05C8A33ULL, 0x91E188A1147E164BULL, 0xE3CD5C114FA2D76AULL, 
        0xA7B8BEC2B58C8F5CULL, 0x1FCF9126A6FD2F39ULL, 0xEA16E7C381AE7C6FULL, 0x31981A4FF477B885ULL, 
        0xF4B35EAD8F4C1877ULL, 0x1D05293EB7333C75ULL, 0x3CDEF5157B7F45E7ULL, 0xAFE07C7376F18A09ULL, 
        0x642404FA4A6663A6ULL, 0x6F9BABEE3582CB6FULL, 0xC99BFB86BE4B039EULL, 0xA699EEDE263D1A61ULL, 
        0x9518AF50033F4583ULL, 0xC5DA33766A476FC1ULL, 0xAA488B6CC72D6E92ULL, 0x6CF48F3CC1632E3EULL, 
        0xD25AE5F83E18123AULL, 0xCB477D1D2ECFDF0ULL, 0x702AE7AFE1B3FF9EULL, 0xB354ADE6B4F8884EULL, 
        0x1B5376FEAD91B7D2ULL, 0x9578E2CF3A20B7A4ULL, 0x3AD853EC202BC70DULL, 0x2CBB419AD4E0C20DULL, 
        0xA20CC82997BEF1D5ULL, 0xC45F5F297EE223BULL, 0x1E210F61B2B2603FULL, 0xA96F796DBDA7E1A3ULL, 
        0x1F7D3BA446CA97BCULL, 0x8681BD22E91BEB48ULL, 0x29B257A82B575646ULL, 0x6CBAF8E5ED3B890EULL, 
        0xBCF4F92933AE735FULL, 0x784FA9156823E13AULL, 0x12EF8373A47B6079ULL, 0x5CCF259B65822D85ULL, 
        0xCF5CE03677CE4784ULL, 0x54F71E7B781CD998ULL, 0x5B16A4E2964CEEA0ULL, 0x7677FD9768843BAFULL}, {
        0x318697F63FD913E8ULL, 0xE7A8460645BF4AA8ULL, 0xB5BBC23D12779330ULL, 0x38A2D567DDEB5FE2ULL, 
        0xBAB8337B101C4037ULL, 0x93F80D6F44C5CACEULL, 0xB81421B2043A2348ULL, 0xC08A13F013105229ULL, 
        0xB9DD6CDBB9E3D1DDULL, 0x31B8DB96A600DCC6ULL, 0xEC3442707A7930A2ULL, 0x131AB3A189F5E928ULL, 
        0xDC40E1F42E6B362ULL, 0x489BC7FB4D298B67ULL, 0x4E6027F4E61F37F8ULL, 0xBD270F7E9980B85ULL, 
        0xB8FDE4010DC75CDCULL, 0x4948509F99BAC717ULL, 0x2D2E919E2ADD942ULL, 0x15918AA5C5153BBFULL, 
        0x1C1B41EBEB470F88ULL, 0xC79D44D07A5437E9ULL, 0x58E28FA15677A1C2ULL, 0xAEE8CE76BA631A0EULL, 
        0xAAE5B99C0FFE9C5EULL, 0x6FAE95564EC1DC83ULL, 0x1B9B3CD14E1661FAULL, 0x3A73834AFBE7B8E6ULL, 
        0x4BA9BB9C6ACB0A0ULL, 0xF78E73E694CF25BCULL, 0x7F09628E34D8DADDULL, 0x1B9E020EB68ABAC5ULL, 
        0x822603011EF4BA47ULL, 0x4FB8FB518E04E432ULL, 0x443EDD7C8B1F1E14ULL, 0x867C80A27AB21EFBULL, 
        0xBE21419976C05FBBULL, 0x57A8C8E767739B8FULL, 0xD73117BB9A94FE62ULL, 0xF14CCEECDE3E15D4ULL, 
        0x27B2EC0879C970ABULL, 0x7A1926F3BB0DDBD4ULL, 0x2C886D4FF95EE3AEULL, 0x26AF108B9EB2DB56ULL, 
        0x84C1F4A4395F3DE5ULL, 0x2002C2F3AFC3844BULL, 0x162EB6D73D7DDFFEULL, 0xD46ADFF54982F278ULL, 
        0xEE6CD780D09D1F16ULL, 0x8B2A22B9D0F740C7ULL, 0x57662775CD0C64ECULL, 0x86BFD622E019BA91ULL, 
        0xC7BCF96B6E761FA5ULL, 0xB1778458AB87FAA5ULL, 0x4F4BB10EE48A9915ULL, 0x9B41E79E156F5974ULL, 
        0x34FA2D9E786D435AULL, 0x7702870472944E95ULL, 0x84CC583FFC8E7D5DULL, 0xA2B263FF59E2DD4BULL, 
        0x8BAB84BA03828FC4ULL, 0x2969A0C03A435AA8ULL, 0x4598F1FD186F516CULL, 0xFF2BEB7062142A64ULL}, {
        0x77823A8CC46268B5ULL, 0xF612A93C26C3B7A6ULL, 0x880696F4C84D2270ULL, 0x541D2045CA636D5DULL, 
        0x3E452FB8294AB26FULL, 0xFC19FE7411D9F3FDULL, 0x826CD60E4DF0E9BBULL, 0x72988D1C1FFD378EULL, 
        0xF2074992565C4DABULL, 0xEB743C5488C4316AULL, 0x58CD94B9C1705F49ULL, 0x9A6F7E8B7AD6BC10ULL, 
        0x6A7E9D9305D6D040ULL, 0x17CF1EDC2A2981DAULL, 0xCB80CBC171A0EC03ULL, 0x7D071C818796DE04ULL, 
        0xF6B0162DE63689EULL, 0xF8EA474D6A5E023BULL, 0x868B3FCAC981CF35ULL, 0x377AADC24F508FDCULL, 
        0xEB1271684A2122DDULL, 0xEF0D3AFA4AFD3333ULL, 0xA2649EDA54C908F2ULL, 0xC0BD3E4BA40F0BEULL, 
        0x4F1A11CE6A80D392ULL, 0x19D9D033CCB602A0ULL, 0xCBE7041D05D30886ULL, 0x5863DD73FAA0266DULL, 
        0x128AD1808E02AB28ULL, 0xA8997FBB31C9A05CULL, 0x60F8D1D93E52447FULL, 0xF6F16D3E381583F2ULL, 
        0x9CB0717130653634ULL, 0xEBD756DC1DEF508CULL, 0x92257E5410D92D52ULL, 0x89C0784A9251A623ULL, 
        0x4EED5D06FBA23374ULL, 0xE095FEAC3A21D676ULL, 0x5A8D47E840A4A123ULL, 0x6220DC931AB82EB9ULL, 
        0x567360D84E99A5D3ULL, 0x2E908E7C8072DDD6ULL, 0x136DDC66914A1707ULL, 0xE0369B6E8712D606ULL, 
        0x7F96BAD4F2A401BAULL, 0xE5DEB7864F062940ULL, 0x23B2DF69E6E4268EULL, 0x53744D703155E321ULL, 
        0xB0093534363FEC32ULL, 0x5EC9D15F29C59FBDULL, 0x7449675C2B096DF7ULL, 0x6AEBD4BCECA11963ULL, 
        0x2223A39FD2337974ULL, 0xA33F63915CB2F0D7ULL, 0xD59073087A1B06FDULL, 0x209A4A71191E3BEULL, 
        0x9107CB3578E2D82FULL, 0xD9E8DBE5AFE5FEBFULL, 0x73DA14306FBB2B2DULL, 0x1E9C79C2C311EF0ULL, 
        0x12ABD03CAE287F48ULL, 0x8EAE8ECB7202433ULL, 0x212B35F9E5231315ULL, 0xC7AB2F4C7E22AFBBULL}, {
        0x3A6704BB3B188DD8ULL, 0x79AE4A67A38ADCB6ULL, 0x184888B53D249B88ULL, 0x95AB673A408228AEULL, 
        0x579ECA564F2AC793ULL, 0x5F680C1B4F991CC7ULL, 0x4EE8F0EC6D1BCADFULL, 0x7F7A856B89A3785EULL, 
        0x315A7D6579057399ULL, 0xDA51E5C8DB2BE495ULL, 0x2D435E0DA900FDD6ULL, 0x881ECB5177DFCD22ULL, 
        0x7AECF0C5C8A0F663ULL, 0x1D216ADFC7056466ULL, 0x4383C401111B2479ULL, 0x4EED56E36A91C53BULL, 
        0x52A89D43F9D3C8DEULL, 0xFFA2EC7711901973ULL, 0x251E69F967FBF3FDULL, 0x885DF7DE81369122ULL, 
        0x8269DC90EA7EEFFEULL, 0x8459F1086A578D7BULL, 0x7CC2616302D2E973ULL, 0xD63272C6E3314556ULL, 
        0x48E51A8BA346A912ULL, 0x97678D07C355A34FULL, 0x376A426CFD24D573ULL, 0xE37A4C18B113AE27ULL, 
        0xB21678930674FC0CULL, 0x1688454B10B48503ULL, 0xC197908A7CA3A64BULL, 0xB9423204C2F0A4A7ULL, 
        0xB26DE36D388C8942ULL, 0x2CC9FA2DB1B44136ULL, 0xE393089A5C83E6DAULL, 0xB349A5E90A4A98C0ULL, 
        0xB5612D86CA21B21CULL, 0x1C2C2E781305CFF7ULL, 0x6AD18763B45383FULL, 0x4F1A9B00D9CBE9A6ULL, 
        0xB578109BD6FC2DA6ULL, 0x91B1D6ACA2008EA6ULL, 0x6AD6782D4BA130A9ULL, 0x4B77166C47311E5ULL, 
        0x8ED8D235365E159AULL, 0x3DF45E3D61B92ULL, 0xA58700DFC47D5F5AULL, 0x65A27F368B5D8C89ULL, 
        0x88D83FF706028AB5ULL, 0xEB53224F575A204ULL, 0x975784ED1E6EC682ULL, 0xED712783B5B8B612ULL, 
        0xA64B282B0A52BE5CULL, 0xC5C5E329BB584B58ULL, 0x1FC286B8B8C3BFCULL, 0xF148508FA637C38BULL, 
        0x7C14573464F24FB5ULL, 0x71A7C66760EC283EULL, 0xC6FF4BA851132C5ULL, 0xDE4B22EB5F9314CULL, 
        0x66DB45CBCA356054ULL, 0x48762A783929D578ULL, 0x2C4E93AE5E1C26EFULL, 0x81EA507EB7DB4519ULL}, {
        0x34C83EC54C0AD054ULL, 0x1E0C0B505901B556ULL, 0x7305DF52BFAAFB96ULL, 0x4AEEE96B193C1A20ULL, 
        0xDBD1002FA21C9E49ULL, 0x789986CF586C5A85ULL, 0xA93ABC766D519EDDULL, 0x5E89D0FF3B907D51ULL, 
        0xCB1A7FC615F8F249ULL, 0x7BFB04BD6C343E8AULL, 0x171BAD4D5E5A1618ULL, 0x61620D7C7187FBC3ULL, 
        0x26DF7084CFC1E6B4ULL, 0x7516DFA2D0429636ULL, 0x77A8A539935F029AULL, 0x80D048EE4E5560FBULL, 
        0xC86A1E8C8206A52FULL, 0x85B8370CDDDC7CEEULL, 0x88F904D32799198BULL, 0xEFD75EF4F83E4769ULL, 
        0x939788F9DCE53A62ULL, 0xF7D663A8AF6674C4ULL, 0x3D9DC71A9E97C54EULL, 0x5716BFF10BE4FE3DULL, 
        0x425834539162DA12ULL, 0x267C3CB1435BC909ULL, 0x3085B63612142CBEULL, 0x8FBDC4278FF5158BULL, 
        0x234CB1D8374F741EULL, 0x75ABCCA450829968ULL, 0x949CB29F35AB7D24ULL, 0x63DA9E3C43102595ULL, 
        0x48AF3BF12356E5BAULL, 0x2EC2E31D1096AAD1ULL, 0xBC847E7E49149FBBULL, 0x80618A2728E4ED53ULL, 
        0x364673916715FC0ULL, 0x1B3632B1CD920834ULL, 0x47AB808D77CF6CB4ULL, 0x404308F2ED56A42ULL, 
        0xA1766A85E7CB20A3ULL, 0x99A0D6125E04DF34ULL, 0xC265D0FD61FE00F8ULL, 0x200C0C93A2AC8448ULL, 
        0x35069A8E326A880AULL, 0x9B64474C0A7C298BULL, 0x79113D467F36AE80ULL, 0x5EAA964AE2EFC697ULL, 
        0x429C52CC859FE5EDULL, 0x74D068F1C74A50FBULL, 0xF09F7808546228CFULL, 0xE22DA2DBBD9D6246ULL, 
        0xD9CF85EC9BE4246FULL, 0x1A64A7EDB979F9D2ULL, 0xEA86AF50CE4ABFF4ULL, 0x478BA8CEAA86DFC4ULL, 
        0xA4C983658ADF6199ULL, 0x3BBC160487B3B4DAULL, 0x10E17642119F3E52ULL, 0x94687FF2C4E91AC3ULL, 
        0x1A355A3E51EC4B6BULL, 0xB75D090EE3DCC9DEULL, 0x7549ED1EC910E26BULL, 0x588DA9935DEFFD94ULL}}};

        static constexpr Hash ztk = 0xC5CA5B65D7FF4C86ULL;
        static constexpr Hash zsd = 0xCDCA90411B794FE1ULL;

        static constexpr char posChars[21] = 
        {'/', '1', '2', '3', '4', '5', '6', '7', '8',
            'k', 'r', 'n', 'q', 'b', 'p', 'K', 'R', 'N', 'Q', 'B', 'P'};

        static constexpr char names[14] = 
        {'k', 'r', 'n', 'q', 'b', 'p', 'x',
			'K', 'R', 'N', 'Q', 'B', 'P', 'X'};

    public:
        Position();

        int countReps();
        void beginZobristHash();
        void eraseHistoryTable();

        int evaluate();
        int evaluateScratch();

        void setStartPos();

        void makeMove(Move, bool);
        void unmakeMove(Move, bool);

        void passMove();
        void unpassMove();

        void sortMoves(int, int);
        int fullMoveGen(int, bool);
        uint64_t perft(int, int);

        std::string makeOpening(int);

        void sendMove(std::string);
        void readFen(std::string);
        std::string makeFen();

        bool getSide();
        int halfMoveCount();

        //more tunables

        static constexpr int mIndx[5] = {0, 9, 24, 33, 38};
        int mobVals[43] =
        {-20, -10, 0, 0, 0, 0, 0, 0, 0, 
        -25, -20, -15, -10, -5, 0, 0, 0, 0, 5, 10, 15, 15, 20, 25,
        -20, -15, -10, -5, 0, 5, 10, 20, 30,
        -15, -10, 0, 5, 10,
        -10, -5, 0, 5, 10};

        int mobValsE[43] = 
        {-50, -40, -20, -10, 0, 10, 20, 30, 35,
        -40, -30, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35, 40,
        -40, -30, -10, 0, 10, 20, 25, 30, 35,
        -15, -10, 0, 5, 10,
        -10, -5, 0, 5, 10};

        //int matVals[6] = {0, 650, 400, 200, 150, 100};

        int mps[6][64] = 
        {{-38, -55, -15, -5, -5, -15, -25, -35, 
        -55, -18, 15, 15, 15, 15, 5, -25, 
        -45, -8, 18, 11, 0, 25, 12, -34, 
        -40, -5, 4, 0, 0, 22, 25, -10, 
        -6, -5, 2, 0, 0, 0, 14, -33, 
        -10, 15, 9, 15, 25, 7, 20, -12, 
        -16, 15, 10, 9, 4, 15, 15, -21, 
        -26, -45, -5, 2, 5, -5, -15, -32},

        {15, 5, -15, -15, -15, -15, -14, 15, 
        2, 15, 39, 15, 17, 26, 15, 28, 
        -5, 3, -15, -15, -15, 9, 1, 11, 
        -4, -8, 15, 15, -15, 15, -6, -11, 
        0, 15, -15, 1, -15, 14, 15, -2, 
        0, 15, 15, -15, 15, -8, 14, -4, 
        -30, -15, -15, -15, -15, 12, -2, -30, 
        -15, -15, 23, 15, 15, 16, 15, -15},

        {-65, -50, -15, -15, -45, -31, -46, -64, 
        -55, -5, -25, -15, 15, -21, -5, -55, 
        -32, -10, 20, -5, -3, 21, 2, -39, 
        -5, 10, 0, 5, 31, 0, 19, -5, 
        -5, 19, 0, 21, 34, 19, -10, -15, 
        -35, 15, -5, 25, 22, 0, 15, -45, 
        -28, -35, -20, -11, 10, -14, -35, -41, 
        -38, -25, -15, -41, -43, -21, -25, -65},

        {-30, -50, -23, -34, -15, -45, -50, -55, 
        -18, 12, -10, -5, -5, -9, -15, -42, 
        -29, 9, 8, 5, 7, 5, 14, -5, 
        -30, 1, 25, 15, 44, 5, 15, -30, 
        -14, -10, 0, 43, 15, 29, 19, -25, 
        -5, -6, -5, 5, 19, 5, -10, -18, 
        -35, -25, -7, 0, 0, -13, -17, -45, 
        -38, -35, -15, -39, -25, -27, -50, -40},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        -10, 0, 0, 15, -5, 0, 0, -10, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 5, 15, 0, 0, 19, 5, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        -15, 0, 0, -5, -5, 0, 0, 7, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, -35, -32, 0, 0, -13, -35, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        6, 5, 12, 15, 15, 10, 10, 5, 
        0, 4, 3, 13, 13, 4, 3, 0, 
        10, -5, -3, 36, 10, 16, -6, -7, 
        -3, -3, 17, 31, 30, 19, -3, -3, 
        0, 4, 27, 35, 19, 15, 1, -1, 
        -20, -25, -10, -15, -15, -19, -25, -20, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        int eps[6][64] = 
        {{-85, -77, -65, -55, -55, -65, -75, -85, 
        -83, -25, -15, -5, -5, -15, -25, -75, 
        -65, -15, 30, 20, 20, 30, -15, -65, 
        -55, -5, 23, 55, 55, 24, -5, -55, 
        -63, -5, 20, 55, 55, 33, -5, -55, 
        -65, -15, 23, 22, 43, 21, -15, -65, 
        -75, -25, -15, -5, -5, -15, -25, -75, 
        -85, -92, -65, -56, -55, -65, -75, -85},

        {15, 6, -14, -12, 10, -13, -15, 15, 
        -10, -5, -2, -4, -3, 1, -5, 11, 
        -15, -15, -15, -15, -15, -15, -13, -15, 
        10, 15, -9, 5, -15, -5, -15, -6, 
        10, 15, -15, -15, -15, -4, 7, 10, 
        10, -13, 2, 12, 15, -5, 13, 10, 
        -10, 8, -15, -15, -14, -15, -14, -20, 
        5, 0, 20, -2, 18, -5, 3, 5},

        {-65, -27, -15, -16, -45, -19, -28, -36, 
        -54, -5, -25, -15, 15, -12, -8, -55, 
        -45, 12, 25, -5, -4, -4, -8, -45, 
        -5, -6, 21, 5, 6, 0, 20, -34, 
        -6, 19, 1, 5, 6, 10, 8, -13, 
        -45, 15, -5, 14, -5, 19, 11, -15, 
        -42, -31, -24, 0, 14, -14, -16, -25, 
        -36, -25, -31, -40, -15, -15, -25, -56},

        {-25, -50, -37, -40, -40, -45, -50, -55, 
        -16, 12, -10, -5, -5, -10, -15, -40, 
        -19, -10, -5, 5, 5, 5, 16, -5, 
        -25, 1, -5, 5, 5, 5, 12, -30, 
        -25, -10, 0, 5, 5, 0, 19, -25, 
        -5, -9, -10, 5, 5, 5, -10, -5, 
        -45, -17, -15, 0, 0, -15, -25, -16, 
        -26, -35, -15, -40, -38, -45, -50, -40},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        -10, 0, 0, -2, -5, 0, 0, -10, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 5, 15, 0, 0, 15, 5, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        -4, 0, 0, -5, -5, 0, 0, 3, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, -27, -26, 0, 0, -29, -35, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        27, 5, 11, 15, 15, 10, 10, 5, 
        15, 4, 3, 13, 13, 4, 3, 0, 
        -4, -5, 6, 10, 10, -5, -6, -7, 
        -3, -3, 5, 10, 10, 5, -3, -3, 
        0, 1, 2, 23, 28, 1, 0, -1, 
        -20, -25, -8, -15, -15, -5, -25, -18, 
        0, 0, 0, 0, 0, 0, 0, 0}};

};