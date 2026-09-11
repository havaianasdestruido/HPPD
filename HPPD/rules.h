#pragma once

// ============================================================
// THE RULES ENGINE
//
// A playful, functional port of a big chunk of the capa rule
// corpus (anti-analysis + host-interaction namespaces). Every
// rule actually runs: detections are real reads/checks, plays
// are short reversible gags. Nothing here installs persistence,
// escalates privileges, injects code, or wipes data - the
// offensive half of the corpus just gets logged as "declined".
//
//   Rules::RunAll()  executes every rule in order and prints
//   a [rule] verdict line per rule (boot-sequence style).
// ============================================================

namespace Rules
{
    void RunAll();
}