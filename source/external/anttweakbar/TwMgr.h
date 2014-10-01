//  ---------------------------------------------------------------------------
//
//  @file       TwMgr.h
//  @brief      Tweak bar manager.
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//
//  note:       Private header
//
//  ---------------------------------------------------------------------------


#if !defined ANT_TW_MGR_INCLUDED
#define ANT_TW_MGR_INCLUDED

#include <AntTweakBar.h>
#define ANT_CALL TW_CALL

#include "TwColors.h"
#include "TwFonts.h"
#include "TwGraph.h"
#include "AntPerfTimer.h"


//#define BENCH // uncomment to activate benchmarks

#ifdef BENCH
#   define PERF(cmd)    cmd
#else   // BENCH
#   define PERF(cmd)
#endif  // BENCH

const int NB_ROTO_CURSORS = 12;


//  ---------------------------------------------------------------------------
//  API unexposed by AntTweakBar.h
//  ---------------------------------------------------------------------------

// bar states -> use TwDefine instead
typedef enum ETwState
{
    TW_STATE_SHOWN       = 1,
    TW_STATE_ICONIFIED   = 2,
    TW_STATE_HIDDEN      = 3,
    TW_STATE_UNICONIFIED = 4,
    TW_STATE_ERROR       = 0
} TwState;
/*ANT_TWEAK_BAR_API*/ int       ANT_CALL TwSetBarState(TwBar *bar, TwState state);
/*ANT_TWEAK_BAR_API*/ //TwState ANT_CALL TwGetBarState(const TwBar *bar);
// var states -> use TwDefine instead: visible/iconified implemented only as string commands
//ANT_TWEAK_BAR_API int     ANT_CALL TwSetVarState(TwBar *bar, const char *name, TwState state);
//ANT_TWEAK_BAR_API TwState ANT_CALL TwGetVarState(const TwBar *bar, const char *name);

struct CTwVarGroup;
typedef void (ANT_CALL *TwStructExtInitCallback)(void *structExtValue, void *clientData);
typedef void (ANT_CALL *TwCopyVarFromExtCallback)(void *structValue, const void *structExtValue, unsigned int structExtMemberIndex, void *clientData);
typedef void (ANT_CALL *TwCopyVarToExtCallback)(const void *structValue, void *structExtValue, unsigned int structExtMemberIndex, void *clientData);
/*ANT_TWEAK_BAR_API*/ TwType    ANT_CALL TwDefineStructExt(const char *name, const TwStructMember *structExtMembers, unsigned int nbExtMembers, size_t structSize, size_t structExtSize, TwStructExtInitCallback structExtInitCallback, TwCopyVarFromExtCallback copyVarFromExtCallback, TwCopyVarToExtCallback copyVarToExtCallback, TwSummaryCallback summaryCallback, void *clientData, const char *help);
typedef void (ANT_CALL *TwCustomDrawCallback)(int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef bool (ANT_CALL *TwCustomMouseMotionCallback)(int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef bool (ANT_CALL *TwCustomMouseButtonCallback)(TwMouseButtonID button, bool pressed, int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef void (ANT_CALL *TwCustomMouseLeaveCallback)(void *structExtValue, void *clientData, TwBar *bar);

enum ERetType 
{
    RET_ERROR = 0,
    RET_DOUBLE,
    RET_STRING
};

enum EButtonAlign   
{ 
    BUTTON_ALIGN_LEFT, 
    BUTTON_ALIGN_CENTER, 
    BUTTON_ALIGN_RIGHT 
};

//  ---------------------------------------------------------------------------
//  AntTweakBar Manager
//  ---------------------------------------------------------------------------

struct CTwMgr
{
  ETwGraphAPI         m_GraphAPI;
  int                 m_WndID;
  int                 m_WndWidth;
  int                 m_WndHeight;
  void *              m_Device;
  class ITwGraph *    m_Graph;
  const CTexFont *    m_CurrentFont;

    std::vector<TwBar*> m_Bars;
    std::vector<int>    m_Order;

    std::vector<bool>   m_MinOccupied;
    void                Minimize(TwBar *_Bar);
    void                Maximize(TwBar *_Bar);
    void                Hide(TwBar *_Bar);
    void                Unhide(TwBar *_Bar);
    void                SetFont(const CTexFont *_Font, bool _ResizeBars);
    int                 m_LastMouseX;
    int                 m_LastMouseY;
    int                 m_LastMouseWheelPos;
    int                 m_IconPos;      // 0: bottom-left, 1:bottom-right, 2:top-left, 3:top-right
    int                 m_IconAlign;    // 0: vertical, 1: horizontal
    int                 m_IconMarginX, m_IconMarginY;
  bool                m_FontResizable;
  bool                m_UseOldColorScheme;
  bool                m_Contained;
  bool                m_OverlapContent;
  std::string         m_BarAlwaysOnTop;
  std::string         m_BarAlwaysOnBottom;
  EButtonAlign        m_ButtonAlign;
  bool                m_Terminating;

  bool                m_HelpBarNotUpToDate;
  bool                m_HelpBarUpdateNow;
  bool                m_InfoBuildText;
  std::string         m_Help;
  TwBar *             m_HelpBar;
  float               m_LastHelpUpdateTime;
  void                UpdateHelpBar();
  float               m_KeyPressedTime;
  void *              m_KeyPressedTextObj;
  std::string         m_KeyPressedStr;
  void *              m_InfoTextObj;

  int                 m_BarInitColorHue;


  int                 FindBar(const char *_Name) const;
  int                 HasAttrib(const char *_Attrib, bool *_HasValue) const;
  int                 SetAttrib(int _AttribID, const char *_Value);
  ERetType            GetAttrib(int _AttribID, std::vector<double>& outDouble, std::ostringstream& outString) const;
  void                SetLastError(const char *_StaticErrorMesssage); // _StaticErrorMesssage must be a static string
  const char *        GetLastError();                                 // returns a static string describing the error, and set LastError to NULL
  const char *        CheckLastError() const;                         // returns the LastError, but does not set it to NULL
  void                SetCurrentDbgParams(const char *file, int line);

  //bool              IsProcessing() const            { return m_Processing);
  //void              SetProcessing(bool processing)  { m_Processing = processing; }

    

  CTwMgr(ETwGraphAPI _GraphAPI, void *_Device, int _WndID);
  ~CTwMgr();
  
  struct CStructMember
  {
    std::string     m_Name;
    std::string     m_Label;
    size_t          m_Offset;
    std::string     m_DefString;
    size_t          m_Size;
    std::string     m_Help;
    TwType          m_Type;
    char dummy[4];
  };


  struct CStruct
  {
    std::string                 m_Name;
    std::vector<CStructMember>  m_Members;
    size_t                      m_Size;
    TwSummaryCallback           m_SummaryCallback;
    void *                      m_SummaryClientData;
    std::string                 m_Help;
    size_t                      m_ClientStructSize;
    TwStructExtInitCallback     m_StructExtInitCallback;
    TwCopyVarFromExtCallback    m_CopyVarFromExtCallback;
    TwCopyVarToExtCallback      m_CopyVarToExtCallback;
    void *                      m_ExtClientData;
  CStruct() : m_StructExtInitCallback(NULL), m_CopyVarFromExtCallback(NULL), m_CopyVarToExtCallback(NULL), m_ExtClientData(NULL), m_IsExt(false) {}
    static void ANT_CALL        DefaultSummary(char *_SummaryString, size_t _SummaryMaxLength, const void *_Value, void *_ClientData);
    static void *               s_PassProxyAsClientData;
    bool                        m_IsExt;
    char dummy[7];
  };
  
  // followings are used for TwAddVarCB( ... StructType ... )
  struct CStructProxy
  {
    void *           m_StructData;
    void *           m_StructExtData;
    TwSetVarCallback m_StructSetCallback;
    TwGetVarCallback m_StructGetCallback;
    void *           m_StructClientData;
    TwCustomDrawCallback        m_CustomDrawCallback;
    TwCustomMouseMotionCallback m_CustomMouseMotionCallback;
    TwCustomMouseButtonCallback m_CustomMouseButtonCallback;
    TwCustomMouseLeaveCallback  m_CustomMouseLeaveCallback;
    TwType           m_Type;
    int              m_CustomIndexFirst;
    int              m_CustomIndexLast;
    bool             m_DeleteStructData;
    bool             m_CustomCaptureFocus;
    char dummy[2];
    CStructProxy();
    ~CStructProxy();
  };

  struct CEnum
  {
    std::string     m_Name;
    typedef std::map<unsigned int, std::string> CEntries;
    CEntries        m_Entries;
  };

  struct CMemberProxy
  {
    CStructProxy *  m_StructProxy;
    struct CTwVar * m_Var;
    struct CTwVarGroup * m_VarParent;
    CTwBar *        m_Bar;
    int             m_MemberIndex;
    char dummy[4];
    CMemberProxy();
    ~CMemberProxy();
    static void ANT_CALL SetCB(const void *_Value, void *_ClientData);
    static void ANT_CALL GetCB(void *_Value, void *_ClientData);
  };
  //void              InitVarData(TwType _Type, void *_Data, size_t _Size);
  //void              UninitVarData(TwType _Type, void *_Data, size_t _Size);

  bool m_KeyPressedBuildText;
  bool                m_CanRepeatMousePressed;
  bool                m_IsRepeatingMousePressed;
  bool                m_CursorsCreated;

  TwBar *             m_PopupBar;  
  std::vector<CEnum>  m_Enums;

  std::list<CStructProxy> m_StructProxies;    // elements should not move
  std::list<CMemberProxy> m_MemberProxies;    // elements should not move
  std::vector<char>   m_CSStringBuffer;
  
  TwType              m_TypeColor32;
  TwType              m_TypeColor3F;
  TwType              m_TypeColor4F;
  TwType              m_TypeQuat4F;
  TwType              m_TypeQuat4D;
  TwType              m_TypeDir3F;
  TwType              m_TypeDir3D;

  struct CCDStdString
  {
    std::string *        m_ClientStdStringPtr;
    char                 m_LocalString[sizeof(std::string)+2*sizeof(void*)]; //+2*sizeof(void*) because of VC++ std::string extra info in Debug
    TwSetVarCallback     m_ClientSetCallback;
    TwGetVarCallback     m_ClientGetCallback;
    void *               m_ClientData;
    static void ANT_CALL SetCB(const void *_Value, void *_ClientData);
    static void ANT_CALL GetCB(void *_Value, void *_ClientData);
  };

  struct CClientStdString  // Convertion between VC++ Debug/Release std::string
  {
    CClientStdString();
    void            FromLib(const char *libStr);
    std::string&    ToClient();
  private:
    char            m_Data[sizeof(std::string)+2*sizeof(void *)];
    std::string     m_LibStr;
  };
  struct CLibStdString   // Convertion between VC++ Debug/Release std::string
  {
    CLibStdString();
        void            FromClient(const std::string& clientStr);
        std::string&    ToLib();
    private:
        char            m_Data[sizeof(std::string)+2*sizeof(void *)];
    };
    struct CCDStdStringRecord 
    {
        void *              m_DataPtr;
        char                m_PrevValue[sizeof(std::string)+2*sizeof(void*)];
        CClientStdString    m_ClientStdString;
    };

  TwMouseButtonID     m_LastMousePressedButtonID;
  std::vector<CStruct> m_Structs;
  std::vector<CCDStdStringRecord> m_CDStdStringRecords;
  std::list<CCDStdString>  m_CDStdStrings;
  void                UnrollCDStdString(std::vector<CCDStdStringRecord>& _Records, TwType _Type, void *_Data);   
  void                RestoreCDStdString(const std::vector<CCDStdStringRecord>& _Records);
  std::map<void *, std::vector<char> > m_CDStdStringCopyBuffers;

    struct CCustom      // custom var type
    {
        virtual         ~CCustom() = 0;
    };
    std::vector<CCustom *> m_Customs;

    PerfTimer           m_Timer;
    double              m_LastMousePressedTime;
    int                 m_LastMousePressedPosition[2];
    double              m_RepeatMousePressedDelay;
    double              m_RepeatMousePressedPeriod;
    double              m_LastDrawTime;

    #if defined(ANT_WINDOWS)
        typedef HCURSOR CCursor;
        CCursor         PixmapCursor(int _CurIdx);
    #elif defined(ANT_UNIX)
        typedef Cursor  CCursor;
        CCursor         PixmapCursor(int _CurIdx);
        Display *       m_CurrentXDisplay;
        Window          m_CurrentXWindow;
    #elif defined(ANT_OSX)
        typedef NSCursor * CCursor;
        CCursor         PixmapCursor(int _CurIdx);
    #endif  // defined(ANT_UNIX)
    void                CreateCursors();
    void                FreeCursors();
    void                SetCursor(CCursor _Cursor);

    CCursor             m_CursorArrow;
    CCursor             m_CursorMove;
    CCursor             m_CursorWE;
    CCursor             m_CursorNS;
    CCursor             m_CursorTopLeft;
    CCursor             m_CursorTopRight;
    CCursor             m_CursorBottomLeft;
    CCursor             m_CursorBottomRight;    
    CCursor             m_CursorHelp;
    CCursor             m_CursorHand;
    CCursor             m_CursorCross;
    CCursor             m_CursorUpArrow;
    CCursor             m_CursorNo;
    CCursor             m_CursorIBeam;
    CCursor             m_RotoCursors[NB_ROTO_CURSORS];
    CCursor             m_CursorCenter;
    CCursor             m_CursorPoint;

    TwCopyCDStringToClient  m_CopyCDStringToClient;
    TwCopyStdStringToClient m_CopyStdStringToClient;
    size_t              m_ClientStdStringStructSize;
    TwType              m_ClientStdStringBaseType;



 protected:
  int                 m_CurrentDbgLine;
  const char *        m_LastError;
  const char *        m_CurrentDbgFile;
  int                 m_NbMinimizedBars;  
  char dummy[4];
  //bool              m_Processing;
  
};

extern CTwMgr *g_TwMgr;


//  ---------------------------------------------------------------------------
//  Extra functions and TwTypes
//  ---------------------------------------------------------------------------


bool TwGetKeyCode(int *_Code, int *_Modif, const char *_String);
bool TwGetKeyString(std::string *_String, int _Code, int _Modif); 

const TwType TW_TYPE_SHORTCUT       = TwType(0xfff1);
const TwType TW_TYPE_HELP_GRP       = TwType(0xfff2);
const TwType TW_TYPE_HELP_ATOM      = TwType(0xfff3);
const TwType TW_TYPE_HELP_HEADER    = TwType(0xfff4);
const TwType TW_TYPE_HELP_STRUCT    = TwType(0xfff5);
const TwType TW_TYPE_BUTTON         = TwType(0xfff6);
const TwType TW_TYPE_CDSTDSTRING    = TwType(0xfff7);
const TwType TW_TYPE_STRUCT_BASE    = TwType(0x10000000);
const TwType TW_TYPE_ENUM_BASE      = TwType(0x20000000);
const TwType TW_TYPE_CSSTRING_BASE  = TW_TYPE_CSSTRING(0);          // defined as 0x30000000 (see AntTweakBar.h)
const TwType TW_TYPE_CSSTRING_MAX   = TW_TYPE_CSSTRING(0xfffffff);
#define TW_CSSTRING_SIZE(type)      ((int)((type)&0xfffffff))
const TwType TW_TYPE_CUSTOM_BASE    = TwType(0x40000000);
const TwType TW_TYPE_STDSTRING_VS2008 = TwType(0x2fff0000);
const TwType TW_TYPE_STDSTRING_VS2010 = TwType(0x2ffe0000);

extern "C" int ANT_CALL TwSetLastError(const char *_StaticErrorMessage);

//const TwGraphAPI TW_OPENGL_CORE = (TwGraphAPI)5; // WIP (note: OpenGL Core Profil requires OpenGL 3.2 or later)

// Clipping helper
struct CRect 
{ 
    int X, Y, W, H;
    CRect() : X(0), Y(0), W(0), H(0) {}
    CRect(int _X, int _Y, int _W, int _H) : X(_X), Y(_Y), W(_W), H(_H) {}
    bool operator==(const CRect& _Rect) { return (Empty() && _Rect.Empty()) || (X==_Rect.X && Y==_Rect.Y && W==_Rect.W && H==_Rect.H); }
    bool Empty(int _Margin=0) const { return (W<=_Margin || H<=_Margin); }
    bool Subtract(const CRect& _Rect, std::vector<CRect>& _OutRects) const;
    bool Subtract(const std::vector<CRect>& _Rects, std::vector<CRect>& _OutRects) const;
};


//  ---------------------------------------------------------------------------
//  Global bar attribs
//  ---------------------------------------------------------------------------


enum EMgrAttribs
{
    MGR_HELP = 1,
    MGR_FONT_SIZE,
    MGR_FONT_STYLE,
    MGR_ICON_POS,
    MGR_ICON_ALIGN,
    MGR_ICON_MARGIN,
    MGR_FONT_RESIZABLE,
    MGR_COLOR_SCHEME,
    MGR_CONTAINED,
    MGR_BUTTON_ALIGN,
    MGR_OVERLAP
};


//  ---------------------------------------------------------------------------
//  Color struct ext
//  ---------------------------------------------------------------------------


struct CColorExt
{
    int                  R, G, B;
    int                  H, L, S;
    int                  A;
    unsigned int         m_PrevConvertedColor;
    CTwMgr::CStructProxy*m_StructProxy;
    void                 RGB2HLS();
    void                 HLS2RGB();
    static void ANT_CALL InitColor32CB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL InitColor3FCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL InitColor4FCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
    static void ANT_CALL CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
    static void ANT_CALL SummaryCB(char *_SummaryString, size_t _SummaryMaxLength, const void *_ExtValue, void *_ClientData);
    static void          CreateTypes();
    bool                 m_HLS, m_HasAlpha, m_OGL;
    bool                 m_CanHaveAlpha;
    bool                 m_IsColorF;
  char dummy[3];
};


//  ---------------------------------------------------------------------------
//  Quaternion struct ext
//  ---------------------------------------------------------------------------


struct CQuaternionExt
{
    double               Qx, Qy, Qz, Qs;    // Quat value
    double               Vx, Vy, Vz, Angle; // Not used
    double               Dx, Dy, Dz;        // Dir value set when used as a direction
    double               m_Dir[3];          // If not zero, display one direction vector
    color32              m_DirColor;        // Direction vector color
    float                m_Permute[3][3];   // Permute frame axis
    CTwMgr::CStructProxy*m_StructProxy;
    static void ANT_CALL InitQuat4FCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL InitQuat4DCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL InitDir3FCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL InitDir3DCB(void *_ExtValue, void *_ClientData);
    static void ANT_CALL CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
    static void ANT_CALL CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
    static void ANT_CALL SummaryCB(char *_SummaryString, size_t _SummaryMaxLength, const void *_ExtValue, void *_ClientData);
    static void ANT_CALL DrawCB(int _W, int _H, void *_ExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
    static bool ANT_CALL MouseMotionCB(int _MouseX, int _MouseY, int _W, int _H, void *_StructExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
    static bool ANT_CALL MouseButtonCB(TwMouseButtonID _Button, bool _Pressed, int _MouseX, int _MouseY, int _W, int _H, void *_StructExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
    static void ANT_CALL MouseLeaveCB(void *_StructExtValue, void *_ClientData, TwBar *_Bar);
    static void          CreateTypes();
    static TwType        s_CustomType;
    void                 ConvertToAxisAngle();
    void                 ConvertFromAxisAngle();
    void                 CopyToVar();
    static std::vector<float>   s_SphTri;
    static std::vector<color32> s_SphCol;
    static std::vector<int>     s_SphTriProj;
    static std::vector<color32> s_SphColLight;
    static std::vector<float>   s_ArrowTri[4];
    static std::vector<int>     s_ArrowTriProj[4];
    static std::vector<float>   s_ArrowNorm[4];
    static std::vector<color32> s_ArrowColLight[4];
    enum EArrowParts     { ARROW_CONE, ARROW_CONE_CAP, ARROW_CYL, ARROW_CYL_CAP };
    static void          CreateSphere();
    static void          CreateArrow();
    static void          ApplyQuat(float *outX, float *outY, float *outZ, float x, float y, float z, float qx, float qy, float qz, float qs);
    static void          QuatFromDir(double *outQx, double *outQy, double *outQz, double *outQs, double dx, double dy, double dz);
    inline void          Permute(float *outX, float *outY, float *outZ, float x, float y, float z);
    inline void          PermuteInv(float *outX, float *outY, float *outZ, float x, float y, float z);
    inline void          Permute(double *outX, double *outY, double *outZ, double x, double y, double z);
    inline void          PermuteInv(double *outX, double *outY, double *outZ, double x, double y, double z);
    double               m_OrigQuat[4];
    float                m_OrigX, m_OrigY;
    double               m_PrevX, m_PrevY;
    bool                 m_AAMode;          // Axis & angle mode -> disabled
    bool                 m_ShowVal;         // Display values
    bool                 m_IsFloat;         // Quat/Dir uses floats
    bool                 m_IsDir;           // Mapped to a dir vector instead of a quat
    bool                 m_Highlighted;
    bool                 m_Rotating;
  char dummy[2];
};


//  ---------------------------------------------------------------------------
//  CTwFPU objects set and restore the fpu precision if needed.
//  (could be useful because DirectX changes it and AntTweakBar requires default double precision)
//  ---------------------------------------------------------------------------


struct CTwFPU
{
    CTwFPU()    
    { 
      /* #ifdef ANT_WINDOWS
        state0 = _controlfp(0, 0); 
        if( (state0&MCW_PC)==_PC_24 )   // we need at least _PC_53
            _controlfp(_PC_53, MCW_PC);
	    #else*/
        state0 = 0;
	//    #endif
    }
    ~CTwFPU()
    {
      /*    #ifdef ANT_WINDOWS      
        if( (state0&MCW_PC)==_PC_24 )
            _controlfp(_PC_24, MCW_PC);
	    #else*/
        state0 = 0;
	//    #endif
    }
private:
    unsigned int state0;
};

//  ---------------------------------------------------------------------------


#endif // !defined ANT_TW_MGR_INCLUDED
