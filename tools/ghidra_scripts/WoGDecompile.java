// Print Ghidra's decompilation of the named functions (to check the project or to read code from WSL).
// Usage (headless, -process cdi_zelda.mod -noanalysis): -postScript WoGDecompile.java <name> [<name> ...]
// @category WandOfGamelon

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;

public class WoGDecompile extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        for (String name : getScriptArgs()) {
            Function f = getGlobalFunctions(name).isEmpty() ? null : getGlobalFunctions(name).get(0);
            if (f == null) {
                println("no function " + name);
                continue;
            }
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            println("==== " + name + "\n" + (r.decompileCompleted() ? r.getDecompiledFunction().getC() : r.getErrorMessage()));
        }
    }
}
