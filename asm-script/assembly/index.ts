export declare namespace vw {
  export function read_feature(ns: string, f: string): f32
  export function write_feature(ns: string, f: string, val: f32): void
}

export function process_example() : void {
  var f = vw.read_feature("f", "t1");
  vw.write_feature("f", "t1000", Math.log(f + 1) as f32);
}
