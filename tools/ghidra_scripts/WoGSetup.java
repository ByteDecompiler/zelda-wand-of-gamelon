// Set up cdi_zelda.mod (imported as a raw 68000 binary at address 0) before Ghidra's auto-analysis:
// function names from config/functions.txt (MacsBug names, fn_/gap_ pieces), MacsBug name bytes marked as
// data, an "A5world" block with the initialized data, register A5 set for all code so A5-relative operands
// become references, and labels for the globals and jump-table entries of config/symbols.txt.
// Run by tools/ghidra_setup.ps1 as a -preScript; argument: the project root folder.
// @category WandOfGamelon

import java.io.File;
import java.math.BigInteger;
import java.nio.file.Files;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.ArrayDataType;
import ghidra.program.model.data.ByteDataType;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.lang.Register;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.SourceType;

public class WoGSetup extends GhidraScript {
    static final long A5 = 0x104000L;          // where A5 points in this listing (any value works)
    static final long A5_BLOCK = 0x100000L;    // A5world block: A5-$4000 .. A5+$1000

    @Override
    public void run() throws Exception {
        File root = new File(getScriptArgs()[0]);
        byte[] mod = Files.readAllBytes(new File(root, "extract/modules/cdi_zelda.mod").toPath());
        Memory mem = currentProgram.getMemory();

        // A5 world, with the initialized data (OS-9 idata: A5 offset, size, bytes) copied in
        long idata = be32(mod, 0x40);
        long idOff = be32(mod, (int) idata), idSize = be32(mod, (int) idata + 4);
        MemoryBlock blk = mem.createInitializedBlock("A5world", toAddr(A5_BLOCK), 0x5000, (byte) 0, monitor, false);
        blk.setWrite(true);
        byte[] init = new byte[(int) idSize];
        System.arraycopy(mod, (int) idata + 8, init, 0, (int) idSize);
        mem.setBytes(toAddr(A5 + idOff), init);
        createLabel(toAddr(A5), "A5", true);

        // A5 is constant for all code
        long codeEnd = idata;
        Register a5 = currentProgram.getRegister("A5");
        currentProgram.getProgramContext().setValue(a5, toAddr(0x52), toAddr(codeEnd - 1), BigInteger.valueOf(A5));

        // Module header and entry point
        createLabel(toAddr(0), "module_header", true);
        setPlateComment(toAddr(0), "OS-9 program module header (size, name, exec, data size, idata, irefs)");
        long exec = be32(mod, 0x30);
        createLabel(toAddr(exec), "_start", true);
        addEntryPoint(toAddr(exec));

        // Functions and MacsBug names
        int nf = 0;
        List<String> lines = Files.readAllLines(new File(root, "config/functions.txt").toPath());
        for (String line : lines) {
            if (line.isBlank() || line.startsWith("#")) continue;
            String[] f = line.trim().split("\\s+");
            long start = Long.parseLong(f[0], 16), end = Long.parseLong(f[1], 16);
            String kind = f[2], name = f[3];
            Address a = toAddr(start);
            if (kind.equals("func")) {
                int n = name.length();
                long nameAt = end - (1 + n + ((1 + n) & 1) + 2);
                createData(toAddr(nameAt), new ArrayDataType(ByteDataType.dataType, (int) (end - nameAt), 1));
                setEOLComment(toAddr(nameAt), "MacsBug name: " + name);
            }
            disassemble(a);
            if (kind.equals("gap")) {
                createLabel(a, name, true);
                setPlateComment(a, name + ": unnamed assembly (library or startup routines)");
            } else {
                if (getFunctionAt(a) == null) createFunction(a, name);
                else getFunctionAt(a).setName(name, SourceType.USER_DEFINED);
                nf++;
            }
        }

        // Globals and jump-table entries
        int ns = 0;
        for (String line : Files.readAllLines(new File(root, "config/symbols.txt").toPath())) {
            String l = line.split("#", 2)[0].trim();
            if (l.isEmpty()) continue;
            String[] s = l.split("\\s+");
            long off = Long.decode(s[2].replace("-0x", "-0X").replace("0x", "0X").replace("0X", "0x"));
            Address a = toAddr(A5 + off);
            createLabel(a, s[0].equals("jt") ? "jt_" + s[1] : s[1], true);
            if (s[0].equals("jt")) setEOLComment(a, "jump-table entry A5+$" + Long.toHexString(off));
            ns++;
        }
        println("WoGSetup: " + nf + " functions, " + ns + " symbols, A5 = 0x" + Long.toHexString(A5));
    }

    static long be32(byte[] b, int i) {
        return ((b[i] & 0xffL) << 24) | ((b[i + 1] & 0xffL) << 16) | ((b[i + 2] & 0xffL) << 8) | (b[i + 3] & 0xffL);
    }
}
