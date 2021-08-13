export declare namespace vw {
  export function read_feature(ns: string, f: string): f32
  export function write_feature(ns: string, f: string, val: f32): void
}

export function process_example() : void {
  var f = vw.read_feature("A", "x");
  vw.write_feature("A", "y", Math.log(f + 1) as f32);
}
