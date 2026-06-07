use proc_macro::TokenStream;
use quote::{format_ident, quote};
use syn::parse::{Parse, ParseStream};
use syn::{braced, Expr, Ident, Token, Type};

#[derive(Clone)]
struct ConfigInput {
    root: StructDef,
}

#[derive(Clone)]
struct StructDef {
    name: Ident,
    fields: Vec<Field>,
}

#[derive(Clone)]
enum Field {
    Leaf {
        name: Ident,
        ty: Box<Type>,
        default: Option<Expr>,
    },
    Nested {
        name: Ident,
        fields: Vec<Field>,
    },
}

impl Parse for ConfigInput {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        let root: StructDef = input.parse()?;
        Ok(ConfigInput { root })
    }
}

impl Parse for StructDef {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        let name: Ident = input.parse()?;
        let content;
        braced!(content in input);
        let mut fields = Vec::new();
        while !content.is_empty() {
            fields.push(content.parse::<Field>()?);
            if content.is_empty() {
                break;
            }
            let _ = content.parse::<Token![,]>();
        }
        Ok(StructDef { name, fields })
    }
}

impl Parse for Field {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        let name: Ident = input.parse()?;
        let _: Token![:] = input.parse()?;

        if input.peek(syn::token::Brace) {
            let content;
            braced!(content in input);
            let mut fields = Vec::new();
            while !content.is_empty() {
                fields.push(content.parse::<Field>()?);
                if content.is_empty() {
                    break;
                }
                let _ = content.parse::<Token![,]>();
            }
            Ok(Field::Nested { name, fields })
        } else {
                let ty: Box<Type> = Box::new(input.parse()?);
            let default = if input.peek(Token![=]) {
                let _: Token![=] = input.parse()?;
                Some(input.parse::<Expr>()?)
            } else {
                None
            };
            Ok(Field::Leaf { name, ty, default })
        }
    }
}

fn pascal_case(name: &Ident) -> String {
    let s = name.to_string();
    let mut result = String::with_capacity(s.len());
    let mut capitalize = true;
    for c in s.chars() {
        if c == '_' {
            capitalize = true;
        } else if capitalize {
            result.push(c.to_ascii_uppercase());
            capitalize = false;
        } else {
            result.push(c);
        }
    }
    result
}

fn gen_structs(def: &StructDef, _parent_name: &Ident) -> proc_macro2::TokenStream {
    let struct_name = &def.name;
    let field_defs: Vec<_> = def
        .fields
        .iter()
        .map(|f| gen_field(f, struct_name))
        .collect();
    let struct_fields: Vec<_> = field_defs.iter().map(|f| &f.0).collect();
    let default_fields: Vec<_> = field_defs.iter().map(|f| &f.1).collect();
    let nested_structs: Vec<_> = def
        .fields
        .iter()
        .filter_map(|f| {
            if let Field::Nested { ref name, ref fields } = f {
                let nested_name = format_ident!(
                    "{}{}",
                    struct_name,
                    pascal_case(name)
                );
                let nested_def = StructDef {
                    name: nested_name,
                    fields: fields.clone(),
                };
                Some(gen_structs(&nested_def, struct_name))
            } else {
                None
            }
        })
        .collect();

    quote! {
        #[derive(Debug, Clone, serde::Deserialize)]
        pub struct #struct_name {
            #(#struct_fields),*
        }

        impl Default for #struct_name {
            fn default() -> Self {
                Self {
                    #(#default_fields),*
                }
            }
        }

        #(#nested_structs)*
    }
}

fn gen_field(field: &Field, parent: &Ident) -> (proc_macro2::TokenStream, proc_macro2::TokenStream) {
    match field {
        Field::Leaf {
            name,
            ty,
            default,
        } => {
            let field_def = quote! { #[serde(default)] pub #name: #ty };
            let default_val = match default {
                Some(expr) => quote! { #expr },
                None => quote! { Default::default() },
            };
            let default_def = quote! { #name: #default_val };
            (field_def, default_def)
        }
        Field::Nested { name, .. } => {
            let nested_type = format_ident!("{}{}", parent, pascal_case(name));
            let field_def = quote! { #[serde(default)] pub #name: #nested_type };
            let default_def = quote! { #name: #nested_type::default() };
            (field_def, default_def)
        }
    }
}

fn gen_load_method(root: &StructDef) -> proc_macro2::TokenStream {
    let name = &root.name;
    quote! {
        impl #name {
            /// Load config from a JSON file.
            ///
            /// If the file does not exist, the default config is written to
            /// the path (creating parent directories as needed) and returned.
            /// If the file exists but contains invalid JSON, a parse error is
            /// returned.
            pub fn load(path: impl AsRef<std::path::Path>) -> Result<Self, helium_wsl::ConfigError> {
                match std::fs::read_to_string(path.as_ref()) {
                    Ok(content) => {
                        serde_json::from_str(&content).map_err(helium_wsl::ConfigError::parsing)
                    }
                    Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                        if let Some(parent) = path.as_ref().parent() {
                            let _ = std::fs::create_dir_all(parent);
                        }
                        let default = Self::default();
                        if let Ok(json) = serde_json::to_string_pretty(&default) {
                            let _ = std::fs::write(path.as_ref(), json);
                        }
                        Ok(default)
                    }
                    Err(e) => Err(helium_wsl::ConfigError::reading(e)),
                }
            }

            /// Save config to a JSON file, creating parent directories if needed.
            pub fn save(&self, path: impl AsRef<std::path::Path>) -> Result<(), helium_wsl::ConfigError> {
                if let Some(parent) = path.as_ref().parent() {
                    std::fs::create_dir_all(parent).map_err(helium_wsl::ConfigError::reading)?;
                }
                let content = serde_json::to_string_pretty(self).map_err(helium_wsl::ConfigError::parsing)?;
                std::fs::write(path.as_ref(), content).map_err(helium_wsl::ConfigError::reading)?;
                Ok(())
            }
        }
    }
}

#[proc_macro]
pub fn helium_config(input: TokenStream) -> TokenStream {
    let config: ConfigInput = syn::parse_macro_input!(input);
    let root = &config.root;

    let structs = gen_structs(root, &root.name);
    let load = gen_load_method(root);

    // We need #[serde(default)] on every field struct-level for nested deserialization
    // We can't easily add it to the struct derive, but we can use #[serde(default)]
    // on each field in the struct definition.

    let expanded = quote! {
        #structs
        #load
    };

    TokenStream::from(expanded)
}
