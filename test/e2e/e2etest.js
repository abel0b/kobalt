import { walk, exists, existsSync } from "https://deno.land/std@0.80.0/fs/mod.ts"
import { assert, assertEquals, assertNotEquals } from "https://deno.land/std@0.80.0/testing/asserts.ts";

const config = Deno.env.get("CONFIG") || "debug"
const exeExt = (Deno.build.os == 'windows') ? '.exe' : ''
const kobaltBin = `./bin/${config}/kobalt${exeExt}`

const NOCOLOR = "-n"
const LEX = "-L"
const PARSE = "-P"
const TYPECHECK = "-T"

async function klrun(path, opt = []) {
    const p = Deno.run({
        cmd: [kobaltBin, NOCOLOR, ...opt, path],
        stdout: "piped",
        stderr: "piped",
    })
    
    const { code } = await p.status()
    const stdout = await p.output()
    const stderr = await p.stderrOutput()
    const report = {
        code,
        stdout: new TextDecoder("utf-8").decode(stdout),
        stderr: new TextDecoder("utf-8").decode(stderr),
    }
    p.close()
    return report
}

async function kltest(path) {
    const tokPath = `${path}tok`
    if (await exists(tokPath)) {
        console.log("oklog " + path)
        Deno.test(`lex ${path}`, async () => {
            const report = await klrun(path, [LEX])
            assertEquals(report.code, 0)
            const expectedTok =  await Deno.readTextFile(tokPath)
            assertEquals(report.stdout, expectedTok)
        })
    }

    const astPath = `${path}ast`
    if (await exists(astPath)) {
        Deno.test(`parse ${path}`, async () => {
            const report = await klrun(path, [PARSE])
            assertEquals(report.code, 0)
            const expectedAst =  await Deno.readTextFile(astPath)
            assertEquals(report.stdout, expectedAst)
        })
    }

    const tastPath = `${path}tast`
    if (await exists(tastPath)) {
        Deno.test(`typecheck ${path}`, async () => {
            const report = await klrun(path, [TYPECHECK])
            assertEquals(report.code, 0)
            const expectedTast = await Deno.readTextFile(tastPath)
            assertEquals(report.stdout, expectedTast)
        })
    }

    const codePath = `${path}code`
    if (await exists(codePath)) {
        Deno.test(`run ${path}`, async () => {
            const report = await klrun(path)
            const expectedCode = Number(await Deno.readTextFile(codePath))
            assertEquals(report.code, expectedCode)
        })
    }
}

Deno.test("kobalt binary exists", async () => {
    assert(await exists(kobaltBin))
})

for await (const entry of walk("test/e2e", { exts: ['.kl'] })) {
    console.log("run test " + entry.path)
    await kltest(entry.path)
}
