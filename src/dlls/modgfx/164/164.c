/*
 * dummya4 (DLL 164 / 0xA4) - empty placeholder DLL.
 *
 * Provides only the three required DLL entry points: a no-op
 * release and initialise, plus a func03 stub that returns 0. No game
 * objects, state, or behaviour live here.
 */

int DummyA4_func03_ret_0(void)
{
    return 0x0;
}

void DummyA4_release(void)
{
}

void DummyA4_initialise(void)
{
}

void* lbl_80318D28[8] = {(void*)0x00000000,  (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         DummyA4_initialise, DummyA4_release,   (void*)0x00000000, DummyA4_func03_ret_0};
