export declare namespace vw {
  export function read_feature(ns: string, f: string): f32
  export function write_feature(ns: string, f: string, val: f32): void
  export function remove_feature(ns: string, f: string): void
}

function rename_feature(ns: string, old_name: string, new_name: string) : void
{
  var value = vw.read_feature(ns, old_name);
  vw.remove_feature(ns, old_name);
  vw.write_feature(ns, new_name, value);
}

//This is the entrypoint to a processing script
export function process_example() : void {
  rename_feature("f", "t1", "t1000");
}
