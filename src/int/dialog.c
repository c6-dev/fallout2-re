#include "int/dialog.h"

#include <string.h>

#include "core.h"
#include "memory_manager.h"
#include "movie.h"
#include "text_font.h"
#include "window_manager.h"

typedef struct STRUCT_56DAE0_FIELD_4_FIELD_C {
    char* field_0;
    union {
        int proc;
        char* string;
    };
    int kind;
    int field_C;
    int field_10;
    int field_14;
    short field_18;
    short field_1A;
} STRUCT_56DAE0_FIELD_4_FIELD_C;

typedef struct STRUCT_56DAE0_FIELD_4 {
    void* field_0;
    char* field_4;
    void* field_8;
    STRUCT_56DAE0_FIELD_4_FIELD_C* field_C;
    int field_10;
    int field_14;
    int field_18; // probably font number
} STRUCT_56DAE0_FIELD_4;

typedef struct STRUCT_56DAE0 {
    Program* field_0;
    STRUCT_56DAE0_FIELD_4* field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
} STRUCT_56DAE0;

typedef struct DialogWindowData {
    short flags;
    int width;
    int height;
    int x;
    int y;
    char* backgroundFileName;
} DialogWindowData;

static STRUCT_56DAE0_FIELD_4* getReply();
static void replyAddOption(const char* a1, const char* a2, int a3);
static void replyAddOptionProc(const char* a1, int a2, int a3);
static void optionFree(STRUCT_56DAE0_FIELD_4_FIELD_C* a1);
static void replyFree();
static int endDialog();
static void printLine(int win, char** strings, int strings_num, int a4, int a5, int a6, int a7, int a8, int a9);
static void printStr(int win, char* a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9);
static int abortReply(int a1);
static void endReply();
static void drawStr(int win, char* a2, int font, int width, int height, int left, int top, int a8, int a9, int a10);

// 0x5184B4
static int tods = -1;

// 0x5184B8
static int topDialogLine = 0;

// 0x5184BC
static int topDialogReply = 0;

// 0x5184E4
DialogFunc1* replyWinDrawCallback = NULL;

// 0x5184E8
DialogFunc2* optionsWinDrawCallback = NULL;

// 0x5184EC
static int defaultBorderX = 7;

// 0x5184F0
static int defaultBorderY = 7;

// 0x5184F4
static int defaultSpacing = 5;

// 0x5184F8
static int replyRGBset = 0;

// 0x5184FC
static int optionRGBset = 0;

// 0x518500
static int exitDialog = 0;

// 0x518504
static int inDialog = 0;

// 0x518508
static int mediaFlag = 2;

// 0x56DAE0
static STRUCT_56DAE0 dialog[4];

// 0x56DB60
static DialogWindowData defaultOption;

// 0x56DB78
static DialogWindowData defaultReply;

// 0x56DB90
static int replyPlaying;

// 0x56DB94
static int replyWin = -1;

// 0x56DB98
static int replyG;

// 0x56DB9C
static int replyB;

// 0x56DBA4
static int optionG;

// 0x56DBA8
static int replyR;

// 0x56DBAC
static int optionB;

// 0x56DBB0
static int optionR;

// 0x56DBB4
static int downButton;

// 0x56DBB8
int dword_56DBB8;

// 0x56DBBC
int dword_56DBBC;

// 0x56DBC0
char* off_56DBC0;

// 0x56DBC4
char* off_56DBC4;

// 0x56DBC8
char* off_56DBC8;

// 0x56DBCC
char* off_56DBCC;

// 0x56DBD0
static char* replyTitleDefault;

// 0x56DBD4
static int upButton;

// 0x56DBD8
int dword_56DBD8;

// 0x56DBDC
int dword_56DBDC;

// 0x56DBE0
char* off_56DBE0;

// 0x56DBE4
char* off_56DBE4;

// 0x56DBE8
char* off_56DBE8;

// 0x56DBEC
char* off_56DBEC;

// 0x42F434
static STRUCT_56DAE0_FIELD_4* getReply()
{
    STRUCT_56DAE0_FIELD_4* v0;
    STRUCT_56DAE0_FIELD_4_FIELD_C* v1;

    v0 = &(dialog[tods].field_4[dialog[tods].field_C]);
    if (v0->field_C == NULL) {
        v0->field_14 = 1;
        v1 = (STRUCT_56DAE0_FIELD_4_FIELD_C*)internal_malloc_safe(sizeof(STRUCT_56DAE0_FIELD_4_FIELD_C), __FILE__, __LINE__); // "..\\int\\DIALOG.C", 789
    } else {
        v0->field_14++;
        v1 = (STRUCT_56DAE0_FIELD_4_FIELD_C*)internal_realloc_safe(v0->field_C, sizeof(STRUCT_56DAE0_FIELD_4_FIELD_C) * v0->field_14, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 793
    }
    v0->field_C = v1;

    return v0;
}

// 0x42F4C0
static void replyAddOption(const char* a1, const char* a2, int a3)
{
    STRUCT_56DAE0_FIELD_4* v18;
    int v17;
    char* v14;
    char* v15;

    v18 = getReply();
    v17 = v18->field_14 - 1;
    v18->field_C[v17].kind = 2;

    if (a1 != NULL) {
        v14 = (char*)internal_malloc_safe(strlen(a1) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 805
        strcpy(v14, a1);
        v18->field_C[v17].field_0 = v14;
    } else {
        v18->field_C[v17].field_0 = NULL;
    }

    if (a2 != NULL) {
        v15 = (char*)internal_malloc_safe(strlen(a2) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 810
        strcpy(v15, a2);
        v18->field_C[v17].string = v15;
    } else {
        v18->field_C[v17].string = NULL;
    }

    v18->field_C[v17].field_18 = windowGetFont();
    v18->field_C[v17].field_1A = defaultOption.flags;
    v18->field_C[v17].field_14 = a3;
}

// 0x42F624
static void replyAddOptionProc(const char* a1, int a2, int a3)
{
    STRUCT_56DAE0_FIELD_4* v5;
    int v13;
    char* v11;

    v5 = getReply();
    v13 = v5->field_14 - 1;

    v5->field_C[v13].kind = 1;

    if (a1 != NULL) {
        v11 = (char*)internal_malloc_safe(strlen(a1) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 830
        strcpy(v11, a1);
        v5->field_C[v13].field_0 = v11;
    } else {
        v5->field_C[v13].field_0 = NULL;
    }

    v5->field_C[v13].proc = a2;

    v5->field_C[v13].field_18 = windowGetFont();
    v5->field_C[v13].field_1A = defaultOption.flags;
    v5->field_C[v13].field_14 = a3;
}

// 0x42F714
static void optionFree(STRUCT_56DAE0_FIELD_4_FIELD_C* a1)
{
    if (a1->field_0 != NULL) {
        internal_free_safe(a1->field_0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 844
    }

    if (a1->kind == 2) {
        if (a1->string != NULL) {
            internal_free_safe(a1->string, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 846
        }
    }
}

// 0x42F754
static void replyFree()
{
    int i;
    int j;
    STRUCT_56DAE0* ptr;
    STRUCT_56DAE0_FIELD_4* v6;

    ptr = &(dialog[tods]);
    for (i = 0; i < ptr->field_8; i++) {
        v6 = &(dialog[tods].field_4[i]);

        if (v6->field_C != NULL) {
            for (j = 0; j < v6->field_14; j++) {
                optionFree(&(v6->field_C[j]));
            }

            internal_free_safe(v6->field_C, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 857
        }

        if (v6->field_8 != NULL) {
            internal_free_safe(v6->field_8, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 860
        }

        if (v6->field_4 != NULL) {
            internal_free_safe(v6->field_4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 862
        }

        if (v6->field_0 != NULL) {
            internal_free_safe(v6->field_0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 864
        }
    }

    if (ptr->field_4 != NULL) {
        internal_free_safe(ptr->field_4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 867
    }
}

// 0x42FB94
static int endDialog()
{
    if (tods == -1) {
        return -1;
    }

    topDialogReply = dialog[tods].field_10;
    replyFree();

    if (replyTitleDefault != NULL) {
        internal_free_safe(replyTitleDefault, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 986
        replyTitleDefault = NULL;
    }

    --tods;

    return 0;
}

// 0x42FC70
static void printLine(int win, char** strings, int strings_num, int a4, int a5, int a6, int a7, int a8, int a9)
{
    int i;
    int v11;

    for (i = 0; i < strings_num; i++) {
        v11 = a7 + i * fontGetLineHeight();
        _windowPrintBuf(win, strings[i], strlen(strings[i]), a4, a5 + a7, a6, v11, a8, a9);
    }
}

// 0x42FCF0
static void printStr(int win, char* a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
    char** strings;
    int strings_num;

    strings = _windowWordWrap(a2, a3, 0, &strings_num);
    printLine(win, strings, strings_num, a3, a4, a5, a6, a7, a8);
    _windowFreeWordList(strings, strings_num);
}

// 0x430104
static int abortReply(int a1)
{
    int result;
    int y;
    int x;

    if (replyPlaying == 2) {
        return _moviePlaying() == 0;
    } else if (replyPlaying == 3) {
        return 1;
    }

    result = 1;
    if (a1) {
        if (replyWin != -1) {
            if (!(mouse_get_buttons() & 0x10)) {
                result = 0;
            } else {
                mouse_get_position(&x, &y);

                if (windowGetAtPoint(x, y) != replyWin) {
                    result = 0;
                }
            }
        }
    }
    return result;
}

// 0x430180
static void endReply()
{
    if (replyPlaying != 2) {
        if (replyPlaying == 1) {
            if (!(mediaFlag & 2) && replyWin != -1) {
                windowDestroy(replyWin);
                replyWin = -1;
            }
        } else if (replyPlaying != 3 && replyWin != -1) {
            windowDestroy(replyWin);
            replyWin = -1;
        }
    }
}

// 0x4301E8
static void drawStr(int win, char* str, int font, int width, int height, int left, int top, int a8, int a9, int a10)
{
    int old_font;
    Rect rect;

    old_font = windowGetFont();
    windowSetFont(font);

    printStr(win, str, width, height, left, top, a8, a9, a10);

    rect.left = left;
    rect.top = top;
    rect.right = width + left;
    rect.bottom = height + top;
    win_draw_rect(win, &rect);
    windowSetFont(old_font);
}

// 0x430D40
int dialogStart(Program* a1)
{
    STRUCT_56DAE0* ptr;

    if (tods == 3) {
        return 1;
    }

    ptr = &(dialog[tods]);
    ptr->field_0 = a1;
    ptr->field_4 = 0;
    ptr->field_8 = 0;
    ptr->field_C = -1;
    ptr->field_10 = -1;
    ptr->field_14 = 1;
    ptr->field_10 = 1;

    tods++;

    return 0;
}

// 0x430DB8
int dialogRestart()
{
    if (tods == -1) {
        return 1;
    }

    dialog[tods].field_10 = 0;

    return 0;
}

// 0x430DE4
int dialogGotoReply(const char* a1)
{
    STRUCT_56DAE0* ptr;
    STRUCT_56DAE0_FIELD_4* v5;
    int i;

    if (tods == -1) {
        return 1;
    }

    if (a1 != NULL) {
        ptr = &(dialog[tods]);
        for (i = 0; i < ptr->field_8; i++) {
            v5 = &(ptr->field_4[i]);
            if (v5->field_4 != NULL && stricmp(v5->field_4, a1) == 0) {
                ptr->field_10 = i;
                return 0;
            }
        }

        return 1;
    }

    dialog[tods].field_10 = 0;

    return 0;
}

// 0x430E84
int dialogTitle(const char* a1)
{
    if (replyTitleDefault != NULL) {
        internal_free_safe(replyTitleDefault, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2561
    }

    if (a1 != NULL) {
        replyTitleDefault = (char*)internal_malloc_safe(strlen(a1) + 1, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2564
        strcpy(replyTitleDefault, a1);
    } else {
        replyTitleDefault = NULL;
    }

    return 0;
}

// 0x430EFC
int dialogReply(const char* a1, const char* a2)
{
    // TODO: Incomplete.
    // _replyAddNew(a1, a2);
    return 0;
}

// 0x430F04
int dialogOption(const char* a1, const char* a2)
{
    if (dialog[tods].field_C == -1) {
        return 0;
    }

    replyAddOption(a1, a2, 0);

    return 0;
}

// 0x430F38
int dialogOptionProc(const char* a1, int a2)
{
    if (dialog[tods].field_C == -1) {
        return 1;
    }

    replyAddOptionProc(a1, a2, 0);

    return 0;
}

// 0x430FD4
int dialogMessage(const char* a1, const char* a2, int timeout)
{
    // TODO: Incomplete.
    return -1;
}

// 0x431088
int dialogGo(int a1)
{
    // TODO: Incomplete.
    return -1;
}

// 0x431184
int dialogGetExitPoint()
{
    return topDialogLine + (topDialogReply << 16);
}

// 0x431198
int dialogQuit()
{
    if (inDialog) {
        exitDialog = 1;
    } else {
        endDialog();
    }

    return 0;
}

// 0x4311B8
int dialogSetOptionWindow(int x, int y, int width, int height, char* backgroundFileName)
{
    defaultOption.x = x;
    defaultOption.y = y;
    defaultOption.width = width;
    defaultOption.height = height;
    defaultOption.backgroundFileName = backgroundFileName;

    return 0;
}

// 0x4311E0
int dialogSetReplyWindow(int x, int y, int width, int height, char* backgroundFileName)
{
    defaultReply.x = x;
    defaultReply.y = y;
    defaultReply.width = width;
    defaultReply.height = height;
    defaultReply.backgroundFileName = backgroundFileName;

    return 0;
}

// 0x431208
int dialogSetBorder(int a1, int a2)
{
    defaultBorderX = a1;
    defaultBorderY = a2;

    return 0;
}

// 0x431218
int dialogSetScrollUp(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7)
{
    upButton = a1;
    dword_56DBD8 = a2;

    if (off_56DBE0 != NULL) {
        internal_free_safe(off_56DBE0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2750
    }
    off_56DBE0 = a3;

    if (off_56DBE4 != NULL) {
        internal_free_safe(off_56DBE4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2752
    }
    off_56DBE4 = a4;

    if (off_56DBE8 != NULL) {
        internal_free_safe(off_56DBE8, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2754
    }
    off_56DBE8 = a5;

    if (off_56DBEC != NULL) {
        internal_free_safe(off_56DBEC, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2756
    }
    off_56DBEC = a5;

    dword_56DBDC = a7;

    return 0;
}

// 0x4312C0
int dialogSetScrollDown(int a1, int a2, char* a3, char* a4, char* a5, char* a6, int a7)
{
    downButton = a1;
    dword_56DBB8 = a2;

    if (off_56DBC0 != NULL) {
        internal_free_safe(off_56DBC0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2765
    }
    off_56DBC0 = a3;

    if (off_56DBC4 != NULL) {
        internal_free_safe(off_56DBC4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2767
    }
    off_56DBC4 = a4;

    if (off_56DBC8 != NULL) {
        internal_free_safe(off_56DBC8, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2769
    }
    off_56DBC8 = a5;

    if (off_56DBCC != NULL) {
        internal_free_safe(off_56DBCC, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2771
    }
    off_56DBCC = a6;

    dword_56DBBC = a7;

    return 0;
}

// 0x431368
int dialogSetSpacing(int value)
{
    defaultSpacing = value;

    return 0;
}

// 0x431370
int dialogSetOptionColor(float a1, float a2, float a3)
{
    optionR = (int)(a1 * 31.0);
    optionG = (int)(a2 * 31.0);
    optionB = (int)(a3 * 31.0);

    optionRGBset = 1;

    return 0;
}

// 0x4313C8
int dialogSetReplyColor(float a1, float a2, float a3)
{
    replyR = (int)(a1 * 31.0);
    replyG = (int)(a2 * 31.0);
    replyB = (int)(a3 * 31.0);

    replyRGBset = 1;

    return 0;
}

// 0x431420
int dialogSetOptionFlags(short flags)
{
    defaultOption.flags = flags;

    return 1;
}

// 0x431420
int dialogSetReplyFlags(short flags)
{
    // FIXME: Obvious error, flags should be set on |defaultReply|.
    defaultOption.flags = flags;

    return 1;
}

// 0x431430
void initDialog()
{
}

// 0x431434
void dialogClose()
{
    if (off_56DBE0) {
        internal_free_safe(off_56DBE0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2818
    }

    if (off_56DBE4) {
        internal_free_safe(off_56DBE4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2819
    }

    if (off_56DBE8) {
        internal_free_safe(off_56DBE8, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2820
    }

    if (off_56DBEC) {
        internal_free_safe(off_56DBEC, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2821
    }

    if (off_56DBC0) {
        internal_free_safe(off_56DBC0, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2823
    }

    if (off_56DBC4) {
        internal_free_safe(off_56DBC4, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2824
    }

    if (off_56DBC8) {
        internal_free_safe(off_56DBC8, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2825
    }

    if (off_56DBCC) {
        internal_free_safe(off_56DBCC, __FILE__, __LINE__); // "..\\int\\DIALOG.C", 2826
    }
}

// 0x431518
int dialogGetDialogDepth()
{
    return tods;
}

// 0x431520
void dialogRegisterWinDrawCallbacks(DialogFunc1* a1, DialogFunc2* a2)
{
    replyWinDrawCallback = a1;
    optionsWinDrawCallback = a2;
}

// 0x431530
int dialogToggleMediaFlag(int a1)
{
    if ((a1 & mediaFlag) == a1) {
        mediaFlag &= ~a1;
    } else {
        mediaFlag |= a1;
    }

    return mediaFlag;
}

// 0x431554
int dialogGetMediaFlag()
{
    return mediaFlag;
}
