// func DebugRegisters
//     PRINT_REG()
// endfunc

_DebugRegisters:
    call PRINT_REG
    leave

// proc ExecuteScript
//     int a
//     a = 10 * 8
//     DebugRegisters()
// endproc

_ExecuteScript:
    enter 1
    imul32 10, 8
    mov32 :sp(-1), r0
    jmp _DebugRegisters
    cls