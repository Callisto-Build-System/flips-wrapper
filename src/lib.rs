use std::{
    fmt::{self, Display, Formatter},
    fs::exists,
    path::{Path, PathBuf},
    process::Command,
};

pub type FResult = Result<Output, FlipsError>;

#[derive(Debug)]
pub struct FlipsWrapper {
    flips_path: PathBuf,
    clean_rom_path: PathBuf,
}

fn vec_u8_to_vec_string(vec: Vec<u8>) -> Vec<String> {
    let utf = String::from_utf8(vec).unwrap();
    let lines: Vec<String> = utf.lines().map(|s| s.to_string()).collect();

    lines
}

impl FlipsWrapper {
    pub fn new<P, C>(flips_path: P, clean_rom_path: C) -> Self
    where
        P: Into<PathBuf>,
        C: Into<PathBuf>,
    {
        FlipsWrapper {
            flips_path: flips_path.into(),
            clean_rom_path: clean_rom_path.into(),
        }
    }

    pub fn create_patch<M, P>(
        &self,
        modified_rom_path: M,
        output_patch_path: Option<P>,
        exact: Option<bool>,
        patch_type: Option<PatchType>,
    ) -> FResult
    where
        M: AsRef<Path>,
        P: AsRef<Path>,
    {
        if !exists(&self.flips_path).unwrap_or(false) {
            return Err(FlipsError::FlipsMissing {
                flips_path: self.flips_path.clone(),
            });
        }

        let mut args: Vec<String> = Vec::new();
        args.push("--create".to_string());

        if let Some(exact) = exact {
            if exact {
                args.push("--exact".to_string());
            }
        }

        if let Some(patch_type) = patch_type {
            args.push(patch_type.to_string());
        }

        args.push(self.clean_rom_path.to_string_lossy().to_string());
        args.push(modified_rom_path.as_ref().to_string_lossy().to_string());

        if let Some(patch_path) = output_patch_path {
            args.push(patch_path.as_ref().to_string_lossy().to_string());
        }

        let cmd = Command::new(&*self.flips_path.to_string_lossy())
            .args(&args)
            .output();

        if let Ok(result) = cmd {
            let output = Output {
                stdout: vec_u8_to_vec_string(result.stdout),
                stderr: vec_u8_to_vec_string(result.stderr),
            };
            if result.status.success() {
                Ok(output)
            } else {
                Err(FlipsError::OperationFailed {
                    output: Some(output),
                    exit_code: result.status.code(),
                })
            }
        } else {
            Err(FlipsError::OperationFailed {
                output: None,
                exit_code: None,
            })
        }
    }

    pub fn apply_patch<P, O>(
        &self,
        patch_path: P,
        output_rom_path: Option<O>,
        exact: Option<bool>,
        ignore_checksum: Option<bool>,
    ) -> FResult
    where
        P: AsRef<Path>,
        O: AsRef<Path>,
    {
        if !exists(&self.flips_path).unwrap_or(false) {
            return Err(FlipsError::FlipsMissing {
                flips_path: self.flips_path.clone(),
            });
        }

        let mut args: Vec<String> = Vec::new();
        args.push("--apply".to_string());

        if let Some(exact) = exact {
            if exact {
                args.push("--exact".to_string());
            }
        }

        if let Some(ignore_checksum) = ignore_checksum {
            if ignore_checksum {
                args.push("--ignore-checksum".to_string());
            }
        }

        args.push(patch_path.as_ref().to_string_lossy().to_string());
        args.push(self.clean_rom_path.to_string_lossy().to_string());

        if let Some(output_rom_path) = output_rom_path {
            args.push(output_rom_path.as_ref().to_string_lossy().to_string());
        }

        let cmd = Command::new(&*self.flips_path.to_string_lossy())
            .args(&args)
            .output();

        if let Ok(result) = cmd {
            let output = Output {
                stdout: vec_u8_to_vec_string(result.stdout),
                stderr: vec_u8_to_vec_string(result.stderr),
            };
            if result.status.success() {
                Ok(output)
            } else {
                Err(FlipsError::OperationFailed {
                    output: Some(output),
                    exit_code: result.status.code(),
                })
            }
        } else {
            Err(FlipsError::OperationFailed {
                output: None,
                exit_code: None,
            })
        }
    }
}

#[derive(Debug)]
pub enum PatchType {
    Ips,
    Bps,
    BpsDelta,
    BpsLinear,
    BPSDeltaMoreMemory,
}

impl Display for PatchType {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let string = match self {
            PatchType::Bps => "--bps",
            PatchType::BpsDelta => "--bps-delta",
            PatchType::BpsLinear => "--bps-linear",
            PatchType::BPSDeltaMoreMemory => "--bps-delta-moremem",
            PatchType::Ips => "--ips",
        };

        write!(f, "{}", string)
    }
}

#[derive(Debug)]
pub struct Output {
    pub stdout: Vec<String>,
    pub stderr: Vec<String>,
}

#[derive(Debug)]
pub enum FlipsError {
    FlipsMissing {
        flips_path: PathBuf,
    },
    OperationFailed {
        output: Option<Output>,
        exit_code: Option<i32>,
    },
}
